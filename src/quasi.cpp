/*******************************************************************
 * **********   quasi - Ellipses: n-layers
 *                      Theory:   approximate
 *                      Results:  efficiency factors
 * Axes 2 and 3 must be linked so that axis 1 moves freely and
 * axes 2 and 2 move together.  This allows us to use analytical
 * formulas for the geometrical factors.
 *
 * Seth M. Morton
 *******************************************************************/

#include <cmath>
#include <complex>
#include "solvers.h"

#define MAXLAYERS 2
#define PI 3.14159265358979323846
#define SQR(x) ((x) * (x))
#define QUAD(x) ((x) * (x) * (x) * (x))

using namespace std;

int quasi (int nlayers,              /* Number of layers */
           complex<double> dielec[], /* Dielectric for the layers */
           double mdie,              /* Dielectric of external medium */
           double rel_rad[][2],      /* Relative radii of the layers */
           double rad[2],            /* Radius of particle */
           double size_param,        /* Size parameter */
           double *extinct,          /* Extinction */
           double *scat,             /* Scattering */
           double *absorb)           /* Absorption */
{

    /*********************
     * Perform some checks
     *********************/

    /* Too many layers for quasistatic approximatio */
    if (nlayers > MAXLAYERS) return 1;

    /*******************************
     *Calculate the relative volumes
     *******************************/

    double tmp[MAXLAYERS][3];
    for (int i = 0; i < nlayers; i++) {
        for (int j = 0; j < 2; j++) {
            if (i == 0) {
                tmp[i][j] = rel_rad[i][j];
            } else {
                tmp[i][j] = 0.0;
            }
        }
    }
    double rel_vol[MAXLAYERS];
    rel_vol[0] = tmp[0][0] * tmp[0][1] * tmp[0][1];
    for (int i = 1; i < nlayers; i++) {
        for (int j = 0; j < i+1; j++) {
            for (int k = 0; k < 3; k++) {
                tmp[i][k] += rel_rad[j][k];
            }
        }
        rel_vol[i] = ( tmp[i][0]   * tmp[i][1]   * tmp[i][1] )
                   - ( tmp[i-1][0] * tmp[i-1][1] * tmp[i-1][1] );
    }

    /***********************************************************
     * Calculate the goemetrical factors for each axis and layer
     ***********************************************************/
   
    double gf[2][MAXLAYERS];
    for (int ilayer = 0; ilayer < nlayers; ilayer++) {

        /* Calculate the absolute radii */
        double radii[2];
        for (int i = 0; i < 2; i++) { radii[i] = rel_rad[ilayer][i] * rad[i]; }

        /* Determine if this is a prolate or oblate spheroid or a sphere */
        if (fabs(radii[0] - radii[1]) < 1E-3) { /* Sphere */

            /* All values are 1/3 */
            for (int i = 0; i < 3; i++) { gf[i][ilayer] = 1.0 / 3.0; }

        } else if (radii[0] > radii[1]) { /* Prolate (cigar) */

            /* Determine long axis */
            double e = 1.0 - ( radii[1] / radii[0] );
            gf[0][ilayer] = ( ( 1.0 - e ) / e )
                          * ( -1.0 + ( 1.0 / ( 2 * sqrt(e) ) )
                             * log(( 1.0 + sqrt(e) ) / ( 1.0 - sqrt(e) ))
                            );

            /* Set short axes.  Total must be 1 */
            gf[1][ilayer] = ( 1.0 - gf[0][ilayer] ) / 2.0;

        } else { /* Oblate (pancake) */

            /* Determine long axes */
            double gf[2][MAXLAYERS];
            double e = 1.0 - ( radii[0] / radii[1] );
            double g = sqrt(( 1.0 - e ) / e);
            gf[1][ilayer] = ( g / ( 2.0 * e ) ) * ( ( PI / 2.0 ) - atan(g) )
                          - ( SQR(g) / 2.0 );

            /* Set short axis.  Total must be 1 */
            gf[0][ilayer] = 1.0 - 2 * gf[1][ilayer];

        }

    }

   /*********************************
    * Determine the system dielectric
    *********************************/

    complex<double> die[2];
    complex<double> a[2];
    a[0] = dielec[0] - mdie;
    a[1] = dielec[1] - mdie;
    /* One layer */
    if (nlayers == 1) {
        for (int i = 0; i < 3; i++) {
            die[i] = a[0] / ( 3.0 * ( mdie + gf[i][0] * a[0] ) );
        }
    /* Two layers */
    } else if (nlayers == 2) {
        /* Numerator and denominator */
        complex<double> num[2], den[2], b = dielec[0] - dielec[1];
        for (int i = 0; i < 2; i++) {
            /* Numerator */
            num[i] = a[1] 
                   * ( dielec[1] + b * ( gf[i][0] - rel_vol[0] * gf[i][1] ) );
            num[i] += rel_vol[0] * dielec[1] * b;
            /* Denominator */
            den[i] = ( dielec[1] + b * ( gf[i][0] - rel_vol[0] * gf[i][1] )  )
                   * ( mdie + ( dielec[1] - mdie ) * gf[i][1] );
            den[i] += rel_vol[0] * gf[i][1] * dielec[1] * b;
            /* The whole thing */
            die[i] = num[i] / ( 3.0 * den[i] );
        }
    }

    /**************************
     * Calculate the properties
     **************************/

    *absorb  = 4.0 * size_param * imag(die[0] + die[1] + die[1]) / 3.0;
    *scat    = abs(( die[0] + die[1] + die[1] ) / 3.0);
    *scat    = ( 8.0 / 3.0 ) * QUAD(size_param) * SQR(*scat);
    *extinct = *absorb + *scat;

    return 0;

}
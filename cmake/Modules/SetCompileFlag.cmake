###########################################################################
# Given a list of flags, this function will try each, one at a time,
# and choose the first flag that works.  If no flags work, then nothing
# will be set, unless the REQUIRED key is given, in which case an error
# will be given.
# 
# Call is:
# SET_COMPILE_FLAG(FLAGVAR FLAGVAL (FORTRAN|C|CXX) <REQUIRED> flag1 flag2...)
###########################################################################

INCLUDE (${CMAKE_ROOT}/Modules/CheckCCompilerFlag.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckCXXCompilerFlag.cmake)

FUNCTION (SET_COMPILE_FLAG FLAGVAR FLAGVAL LANG)

    # Make a variable holding the flags.  Filter out REQUIRED if it is there
    SET(FLAG_REQUIRED FALSE)
    SET(FLAG_FOUND FALSE)
    UNSET(FLAGLIST)
    FOREACH (var ${ARGN})
        STRING(TOUPPER "${var}" UP)
        IF(UP STREQUAL "REQUIRED")
            SET(FLAG_REQUIRED TRUE)
        ELSE()
            SET(FLAGLIST ${FLAGLIST} "${var}")
        ENDIF(UP STREQUAL "REQUIRED")
    ENDFOREACH (var ${ARGN})

    # Now, loop over each flag
    FOREACH(flag ${FLAGLIST})

        UNSET(FLAG_WORKS)
        # Check the flag for the given language
        IF(LANG STREQUAL "C")
            CHECK_C_COMPILER_FLAG("${flag}" FLAG_WORKS)
        ELSEIF(LANG STREQUAL "CXX")
            CHECK_CXX_COMPILER_FLAG("${flag}" FLAG_WORKS)
        ELSEIF(LANG STREQUAL "FORTRAN")
            # There is no nice function to do this for FORTRAN, so we must manually
            # create a test program and check if it compiles with a given flag
            SET(TESTFILE ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY})
            SET(TESTFILE ${TESTFILE}/CMakeTmp/testFortranFlags.f90)
            # Generate a test code
            FILE (WRITE ${TESTFILE}
"
program dummyprog
  i = 5
end program dummyprog
")
            # Check that this code compiles with the given flag
            SET(TESTFILE CMakeTmp/testFortranFlags.f90)
            TRY_COMPILE (FLAG_WORKS ${CMAKE_BINARY_DIR} ${TESTFILE}
                COMPILE_DEFINITIONS "${flag}")
        ELSE()
            MESSAGE(FATAL_ERROR "Unknown language in SET_COMPILE_FLAGS: ${LANG}")
        ENDIF(LANG STREQUAL "C")

        # If this worked, use these flags, otherwise use other flags
        IF (FLAG_WORKS)
            # Append this flag to the end of the list that already exists
            SET(${FLAGVAR} "${FLAGVAL} ${flag}" CACHE STRING
                 "Set the ${FLAGVAR} flags" FORCE)
            SET(FLAG_FOUND TRUE)
            BREAK() # We found something that works, so exit
        ENDIF (FLAG_WORKS)

    ENDFOREACH(flag ${FLAGLIST})

    # Raise an error if no flag was found
    IF(FLAG_REQUIRED AND NOT FLAG_FOUND)
        MESSAGE(FATAL_ERROR "No compile flags were found")
    ENDIF(FLAG_REQUIRED AND NOT FLAG_FOUND)

ENDFUNCTION ()

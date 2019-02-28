# Check for the presence of the ATS9462 digitizer
#
# The following variables are set when ATS9462 is found:
#  HAVE_ATS9462          = Set to true, if all components of ATS9462
#                          have been found.
#  ATS9462_INCLUDE_DIR   = Include path for the header files of ATS9462
#  ATS9462_LIBRARIES     = Link these to use ATS9462

## -----------------------------------------------------------------------------
## Check for the header files

find_path (ATS9462_INCLUDE_DIR NAMES AlazarApi.h AlazarCmd.h AlazarDSP.h AlazarError.h AlazarRC.h
  PATHS ${ATS_SDK_PREFIX} /usr/local/include /usr/include /sw/include
)

## -----------------------------------------------------------------------------
## Check for the library

find_library (ATS9462_LIBRARIES ATSApi
  PATHS ${ATS_SDK_PREFIX} /usr/local/lib /usr/lib /lib /sw/lib
)

## -----------------------------------------------------------------------------
## Actions taken when all components have been found

if (ATS9462_INCLUDE_DIR AND ATS9462_LIBRARIES)
  set (HAVE_ATS9462 TRUE)
else (ATS9462_INCLUDE_DIR AND ATS9462_LIBRARIES)
  if (NOT ATS9462_FIND_QUIETLY)
    if (NOT ATS9462_INCLUDE_DIR)
      message (STATUS "Unable to find ATS9462 header files!")
    endif (NOT ATS9462_INCLUDE_DIR)
    if (NOT ATS9462_LIBRARIES)
      message (STATUS "Unable to find ATS9462 library files!")
    endif (NOT ATS9462_LIBRARIES)
  endif (NOT ATS9462_FIND_QUIETLY)
endif (ATS9462_INCLUDE_DIR AND ATS9462_LIBRARIES)

if (HAVE_ATS9462)
  if (NOT ATS9462_FIND_QUIETLY)
    message (STATUS "Found components for ATS9462")
    message (STATUS "ATS9462_INCLUDE_DIR = ${ATS9462_INCLUDE_DIR}")
    message (STATUS "ATS9462_LIBRARIES = ${ATS9462_LIBRARIES}")
  endif (NOT ATS9462_FIND_QUIETLY)
else (HAVE_ATS9462)
  if (ATS9462_FIND_REQUIRED)
    message (FATAL_ERROR "Could not find ATS9462!")
  endif (ATS9462_FIND_REQUIRED)
endif (HAVE_ATS9462)

mark_as_advanced (
  HAVE_ATS9462
  ATS9462_LIBRARIES
  ATS9462_INCLUDE_DIR
)
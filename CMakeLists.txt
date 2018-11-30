# Minimum cmake verison 3.1 required for the variable CMAKE_CXX_STANDARD
cmake_minimum_required (VERSION 3.1)

# Define the project
cmake_policy( SET CMP0048 NEW ) # version in project()
project( FastDaq VERSION 0.1.0 )

list( APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/psyllid/midge/scarab/cmake )
include( PackageBuilder )

list( APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/psyllid/midge/cmake )
include( MidgeUtility )

list( APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/psyllid/cmake )
include( Psyllid )

pbuilder_prepare_project()


#########
# flags #
#########

set_option( USE_CPP11 TRUE )

################
# dependencies #
################

# AlazarTech
list( APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake )
include( ATS9462 )
add_ats9462( ATS_FOUND )
option( FastDAQ_ENABLE_ATS "Flag to enable building of node to read from AlazarTech digitizer" ${ATS_FOUND} )

# FFTW
option( FastDAQ_ENABLE_FFTW "Flag to enable FFTW features" TRUE )
if (FastDAQ_ENABLE_FFTW)
    find_package(FFTW REQUIRED)
else (FastDAQ_ENABLE_FFTW)
    set (FFTW_FOUND FALSE)
endif (FastDAQ_ENABLE_FFTW)
if (FFTW_FOUND)
    add_definitions(-DFFTW_FOUND)
    pbuilder_add_ext_libraries (${FFTW_LIBRARIES})
    if (FFTW_THREADS_FOUND)
        set (FFTW_NTHREADS 1 CACHE STRING "Number of threads to use for FFTW processes")
        add_definitions (-DFFTW_NTHREADS=${FFTW_NTHREADS})
        message (STATUS "FFTW configured to use up to ${FFTW_NTHREADS} threads.")
    else (FFTW_THREADS_FOUND)
        remove_definitions (-DFFTW_NTHREADS=${FFTW_NTHREADS})
    endif (FFTW_THREADS_FOUND)
else (FFTW_FOUND)
    message(STATUS "Building without FFTW")
    remove_definitions(-DFFTW_FOUND)
    remove_definitions (-DFFTW_NTHREADS=${FFTW_NTHREADS})
endif (FFTW_FOUND)
include_directories (${FFTW_INCLUDE_DIR})

# psyllid
pbuilder_add_submodule( Psyllid psyllid )

# need definitions from Monarch
set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} APPEND PROPERTY COMPILE_DEFINITIONS ${Monarch_COMPILE_DEFINITIONS} )


############
# fast_daq #
############

# add include directories
include_directories( BEFORE
    source/daq
)

# build this project
add_subdirectory( source/daq )

# build psyllid, linking against fast_daq
psyllid_build_executables()

#add_subdirectory( source/applications )
#add_subdirectory( source/test )

pbuilder_variables_for_parent()

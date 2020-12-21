# Minimum cmake verison 3.1 required for the variable CMAKE_CXX_STANDARD
cmake_minimum_required (VERSION 3.9)

# Define the project
cmake_policy( SET CMP0048 NEW ) # version in project()
project( FastDaq VERSION 0.1.0 )

# Setup the module path in the proper order for overriding dependencies
list( APPEND CMAKE_MODULE_PATH 
    ${PROJECT_SOURCE_DIR}/sandfly/cmake 
    ${PROJECT_SOURCE_DIR}/cmake 
)
include( sandfly )

pbuilder_prepare_project()


#########
# flags #
#########

option( FastDAQ_ENABLE_ATS "Flag to enable building of node to read from AlazarTech digitizer" )
option( FastDAQ_ENABLE_FFTW "Flag to enable FFTW features" TRUE )
set_option( USE_CPP11 TRUE )

set_option( Midge_ENABLE_EXECUTABLES FALSE )
set_option( Sandfly_ENABLE_EXECUTABLES FALSE )


################
# dependencies #
################

set( PUBLIC_EXT_LIBS )

# AlazarTech
if (FastDAQ_ENABLE_ATS)
    find_package(ATS9462 REQUIRED)
else (FastDAQ_ENABLE_ATS)
    set (ATS9462_FOUND FALSE)
endif (FastDAQ_ENABLE_ATS)
if (ATS9462_FOUND)
    list( APPEND PUBLIC_EXT_LIBS ${ATS9462_LIBRARIES} )
    include_directories (${ATS9462_INCLUDE_DIR})
    add_definitions( -DATS_ENABLED )
endif (ATS9462_FOUND)

# FFTW
if (FastDAQ_ENABLE_FFTW)
    find_package(FFTW COMPONENTS SINGLE REQUIRED)
else (FastDAQ_ENABLE_FFTW)
    set (FFTW_FOUND FALSE)
endif (FastDAQ_ENABLE_FFTW)
if (FFTW_FOUND)
    add_definitions(-DFFTW_FOUND)
    list( APPEND PUBLIC_EXT_LIBS ${FFTW_LIBRARIES} )
    if (FFTW_THREADS_FOUND)
        list( APPEND PUBLIC_EXT_LIBS ${FFTW_THREADS_LIBRARIES} )
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

# Sandfly
add_definitions( -DDRIPLINE_AUTH_FILE=~/.project8_authentications.json )
pbuilder_add_submodule( Sandfly sandfly )

# Monarch
pbuilder_add_submodule( Monarch monarch )

############
# fast_daq #
############

# add include directories
include_directories( BEFORE
    source/utility
    source/control
    source/data
    source/daq
)

# build this project
add_subdirectory( source/utility )
add_subdirectory( source/control )
add_subdirectory( source/data )
add_subdirectory( source/daq )

#add_subdirectory( source/applications )
#add_subdirectory( source/test )
configure_file( ${PROJECT_SOURCE_DIR}/FastDaqConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/FastDaqConfig.cmake @ONLY     )

pbuilder_do_package_config()

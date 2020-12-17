###########
# utility #
###########

set( headers
    fast_daq_error.hh
    fast_daq_version.hh
)
set( sources
    fast_daq_error.cc
)

configure_file( fast_daq_version.cc.in ${CMAKE_CURRENT_BINARY_DIR}/fast_daq_version.cc )
set( sources
    ${sources}
    ${CMAKE_CURRENT_BINARY_DIR}/fast_daq_version.cc
)


###########
# library #
###########

pbuilder_library(
	TARGET FastDaqUtility
    SOURCES ${sources}
)

pbuilder_install_headers( ${headers} )
pbuilder_use_sm_library(SandflyUtility Sandfly)

###########
# control #
###########

set( headers
    butterfly_house.hh
    daq_control.hh
    fast_daq_config.hh
    monarch3_wrap.hh
)

set( sources
    butterfly_house.cc
    daq_control.cc
    fast_daq_config.cc
    monarch3_wrap.cc
)

set( dependencies
    FastDaqUtility
)

###########
# library #
###########

pbuilder_use_sm_library(SandflyControl Sandfly)
pbuilder_use_sm_library(Monarch3 Monarch)
pbuilder_library(
    TARGET FastDaqControl
    SOURCES ${sources}
    PROJECT_LIBRARIES ${dependencies}
)

pbuilder_install_headers( ${headers} )



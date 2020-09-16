###########
# control #
###########

set( headers
    butterfly_house.hh
    daq_control.hh
    monarch3_wrap.hh
)

set( sources
    butterfly_house.cc
    daq_control.cc
    monarch3_wrap.cc
)

set( dependencies
    Fast_daqUtility
)

###########
# library #
###########

pbuilder_library(
    TARGET Fast_daqControl
    SOURCES ${sources}
    PROJECT_LIBRARIES ${dependencies}
)

pbuilder_install_headers( ${headers} )

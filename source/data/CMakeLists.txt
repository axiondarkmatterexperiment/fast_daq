########
# data #
########

set( headers
    frequency_data.hh
    iq_time_data.hh
    power_data.hh
    real_time_data.hh
)

set( sources
    frequency_data.cc
    iq_time_data.cc
    power_data.cc
    real_time_data.cc
)

set ( dependencies
    FastDaqUtility
)

###########
# library #
###########

pbuilder_use_sm_library(SandflyUtility Sandfly)
pbuilder_library(
    TARGET FastDaqData
    SOURCES ${sources}
    PROJECT_LIBRARIES ${dependencies}
)

pbuilder_install_headers( ${headers} )

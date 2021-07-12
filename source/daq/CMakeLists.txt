#######
# daq #
#######
set( headers
    ats_streaming_writer.hh
    data_producer.hh
    dead_end.hh
    egg_writer.hh
    frequency_transform.hh
    inverse_frequency_transform.hh
    power_averager.hh
    spectrum_relay.hh
    streaming_frequency_writer.hh
)

set( sources
    ats_streaming_writer.cc
    data_producer.cc
    dead_end.cc
    egg_writer.cc
    frequency_transform.cc
    inverse_frequency_transform.cc
    power_averager.cc
    spectrum_relay.cc
    streaming_frequency_writer.cc
)

if( ATS9462_FOUND )
    # digitizer node
    set( headers ${headers} ATS9462_digitizer.hh )
    set( sources ${sources} ATS9462_digitizer.cc )
endif( ATS9462_FOUND )

set( dependencies
    FastDaqData
    FastDaqControl
    FastDaqUtility
)

###########
# library #
###########

pbuilder_use_sm_library(SandflyUtility Sandfly)
pbuilder_use_sm_library(Monarch3 Monarch)
pbuilder_library(
    TARGET FastDaqDAQ
    SOURCES ${sources}
    PROJECT_LIBRARIES ${dependencies}
    PUBLIC_EXTERNAL_LIBRARIES ${PUBLIC_EXT_LIBS}
    PRIVATE_EXTERNAL_LIBRARIES ${PRIVATE_EXT_LIBS}
)

pbuilder_install_headers( ${headers} )

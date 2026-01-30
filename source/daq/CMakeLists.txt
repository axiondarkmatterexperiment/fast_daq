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
    packet_receiver_socket.hh
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
    packet_receiver_socket.cc
    power_averager.cc
    spectrum_relay.cc
    streaming_frequency_writer.cc
)

if( FastDAQ_ENABLE_FPA )
    set( headers
        ${headers}
        packet_receiver_fpa.hh
    )

    set( sources
        ${sources}
        packet_receiver_fpa.cc
    )
endif()

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

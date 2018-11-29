#######
# daq #
#######
set( headers
    event_builder.hh
    frequency_mask_trigger.hh
    frequency_transform.hh
    packet_receiver_socket.hh
    roach_config.hh
    terminator.hh
    tf_roach_monitor.hh
    tf_roach_receiver.hh
)

set( sources
    event_builder.cc
    frequency_mask_trigger.cc
    frequency_transform.cc
    packet_receiver_socket.cc
    roach_config.cc
    terminator.cc
    tf_roach_monitor.cc
    tf_roach_receiver.cc
)

option( Psyllid_ENABLE_STREAMED_FREQUENCY_OUTPUT "Flag to enable building node for streaming frequency data (inproper monarch usage)" FALSE )
if (Psyllid_ENABLE_STREAMED_FREQUENCY_OUTPUT)
    set( headers ${sources} streaming_frequency_writer.hh)
    set( sources ${sources} streaming_frequency_writer.cc)
endif (Psyllid_ENABLE_STREAMED_FREQUENCY_OUTPUT)

if( Psyllid_BUILD_FPA )
    set( headers
        ${headers}
        packet_receiver_fpa.hh
    )

    set( sources
        ${sources}
        packet_receiver_fpa.cc
    )
endif( Psyllid_BUILD_FPA )

set( dependencies
)

###########
# library #
###########

midge_library( FastDAQ sources dependencies )
pbuilder_install_headers( ${headers} )

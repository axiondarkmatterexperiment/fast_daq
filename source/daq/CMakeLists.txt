#######
# daq #
#######
set( headers
    frequency_transform.hh
)

set( sources
    frequency_transform.cc
)

option( Psyllid_ENABLE_STREAMED_FREQUENCY_OUTPUT "Flag to enable building node for streaming frequency data (inproper monarch usage)" FALSE )
if (Psyllid_ENABLE_STREAMED_FREQUENCY_OUTPUT)
    set( headers ${sources} streaming_frequency_writer.hh)
    set( sources ${sources} streaming_frequency_writer.cc)
endif (Psyllid_ENABLE_STREAMED_FREQUENCY_OUTPUT)

###########
# library #
###########

midge_library( FastDAQ sources dependencies )
pbuilder_install_headers( ${headers} )

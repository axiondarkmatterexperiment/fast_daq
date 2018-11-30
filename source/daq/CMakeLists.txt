#######
# daq #
#######
set( headers
    ATS9462_digitizer.hh
    frequency_transform.hh
)

set( sources
    ATS9462_digitizer.cc
    frequency_transform.cc
)

option( Psyllid_ENABLE_STREAMED_FREQUENCY_OUTPUT "Flag to enable building node for streaming frequency data (inproper monarch usage)" FALSE )
if (Psyllid_ENABLE_STREAMED_FREQUENCY_OUTPUT)
    set( headers ${headers} streaming_frequency_writer.hh)
    set( sources ${sources} streaming_frequency_writer.cc)
endif (Psyllid_ENABLE_STREAMED_FREQUENCY_OUTPUT)

if ( FastDAQ_ENABLE_ATS)
    #set( headers ${headers} ATS9462_digitizer.hh )
    #set( sources ${sources} ATS9462_digitizer.cc )
    message( STATUS "%%%%%%%%% fast DAQ enabled" )
else ( FastDAQ_ENABLE_ATS)
    message( STATUS "%%%%%%%%% fast DAQ not enabled" )
endif ( FastDAQ_ENABLE_ATS)

###########
# library #
###########

midge_library( FastDAQ sources dependencies )
pbuilder_install_headers( ${headers} )

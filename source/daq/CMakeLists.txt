#######
# daq #
#######
set( headers
    dead_end.hh
    frequency_transform.hh
    inverse_frequency_transform.hh
    power_averager.hh
    spectrum_relay.hh
)

set( sources
    dead_end.cc
    frequency_transform.cc
    inverse_frequency_transform.cc
    power_averager.cc
    spectrum_relay.cc
)

if ( FastDAQ_ENABLE_ATS)
    # digitizer node
    set( headers ${headers} ATS9462_digitizer.hh )
    set( sources ${sources} ATS9462_digitizer.cc )
endif ( FastDAQ_ENABLE_ATS)

set( dependencies
    FastDaqData
)

###########
# library #
###########

midge_library( FastDAQ sources dependencies )
pbuilder_install_headers( ${headers} )

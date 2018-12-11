########
# data #
########

set( headers
    frequency_data.hh
    real_time_data.hh
)

set( sources
    frequency_data.cc
    real_time_data.cc
)

#set( dependencies
#    PsyllidUtility
#)

###########
# library #
###########

midge_library( FastDaqData sources dependencies )
pbuilder_install_headers( ${headers} )



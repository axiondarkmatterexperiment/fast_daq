########
# data #
########

set( headers
    real_time_data.hh
)

set( sources
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



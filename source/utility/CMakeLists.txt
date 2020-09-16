###########
# utility #
###########

set( headers
    byte_swap.hh
    sandfly_error.hh
    sandfly_version.hh
)
set( sources
    sandfly_error.cc
)

configure_file( sandfly_version.cc.in ${CMAKE_CURRENT_BINARY_DIR}/sandfly_version.cc )
set( sources
    ${sources}
    ${CMAKE_CURRENT_BINARY_DIR}/sandfly_version.cc
)


###########
# library #
###########

pbuilder_library(
	TARGET SandflyUtility
    SOURCES ${sources}
)

pbuilder_install_headers( ${headers} )

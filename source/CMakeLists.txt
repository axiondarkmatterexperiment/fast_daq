# CMakeLists.txt for fast_daq/library

pbuilder_use_sm_library( SandflyUtility Sandfly )
pbuilder_use_sm_library( SandflyControl Sandfly )
pbuilder_use_sm_library( Monarch3 Monarch )

add_subdirectory( utility )
add_subdirectory( control )
add_subdirectory( data )
add_subdirectory( daq )

pbuilder_component_install_and_export(
    COMPONENT Library
    LIBTARGETS FastDaqUtility FastDaqControl FastDaqData FastDaqDAQ
)

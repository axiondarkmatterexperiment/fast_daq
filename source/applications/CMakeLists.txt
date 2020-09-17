################
# applications #
################

set( lib_dependencies
    FastDaqUtility
    FastDaqData
    FastDaqDAQ
    FastDaqControl
)

set( fast_daq_exe_PROGRAMS )


# Non-fast_daq executables
set( programs
    grab_packet
    #psyllid
)

pbuilder_executables( 
    SOURCES ${programs}
    TARGETS_VAR fast_daq_exe_PROGRAMS
    PROJECT_LIBRARIES ${lib_dependencies}
    PUBLIC_EXTERNAL_LIBRARIES ${PUBLIC_EXT_LIBS}
    PRIVATE_EXTERNAL_LIBRARIES ${PRIVATE_EXT_LIBS}
)


# FastDaq executable
build_sandfly_executable(
    ALT_NAME fast_daq
    ALT_SOURCES fast_daq.cc
    SANDFLY_SUBMODULE_NAME Sandfly
    PROJECT_LIBRARIES ${lib_dependencies}
)

list( APPEND fast_daq_exe_PROGRAMS fast_daq )


# Export
pbuilder_component_install_and_export( 
    COMPONENT Executables
    EXETARGETS ${fast_daq_exe_PROGRAMS}
)

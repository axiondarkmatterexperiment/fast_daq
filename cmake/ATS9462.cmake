macro( add_ats9462 ats_found )
    set( ats_headers AlazarApi.h;AlazarCmd.h;AlazarDSP.h;AlazarError.h;AlazarRC.h )
    set( ${ats_found} TRUE )
    FOREACH( a_header ${ats_headers})
        find_path( FOUND_DIR ${a_header} PATHS ${ATS_SDK_PREFIX} )
        if( NOT FOUND_DIR )
            message( FATAL_ERROR "did not find ats_sdk header <${a_header}>; set ATS_SDK_PREFIX to suggest a path" )
            set( ${ats_found} FALSE)
        else( NOT FOUND_DIR )
            include_directories( ${FOUND_DIR} )
        endif( NOT FOUND_DIR )
    ENDFOREACH()
    find_library( ATS_LIBS ATSApi PATHS ${ATS_SDK_PREFIX} )
    if( NOT ATS_LIBS )
        message( FATAL_ERROR "unable to find libATSApi.so" )
        set( ${ats_found} FALSE)
    else( NOT ATS_LIBS )
        pbuilder_add_ext_libraries( ${ATS_LIBS} )
    endif( NOT ATS_LIBS )
endmacro()

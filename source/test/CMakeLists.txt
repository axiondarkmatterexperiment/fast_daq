# CMakeLists.txt for fast_daq/source/test

# Catch2: prefer the system package (Debian 12/13 ship catch2 3.x) so CI does not
# git-clone on every build; fall back to FetchContent for other environments.
find_package( Catch2 3 QUIET )
if( NOT Catch2_FOUND )
    include( FetchContent )
    FetchContent_Declare( Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.7.1
    )
    FetchContent_MakeAvailable( Catch2 )
endif()

set( test_sources
    test_data_classes.cc
    test_data_producer.cc
    test_dead_end.cc
    test_power_averager.cc
    test_power_averager_binding.cc
)

# Gate on FFTW_FOUND, not the option: FFTW_FOUND is what determines linkability.
if( FFTW_FOUND )
    list( APPEND test_sources test_frequency_transform.cc )
endif()

set( test_programs )
pbuilder_executables(
    SOURCES ${test_sources}
    TARGETS_VAR test_programs
    PROJECT_LIBRARIES FastDaqDAQ FastDaqData FastDaqControl FastDaqUtility
    PUBLIC_EXTERNAL_LIBRARIES Catch2::Catch2WithMain
)

# Register with CTest. Use the TARGET as COMMAND so CMake substitutes the real
# binary path -- a bare output-name string would depend on PATH.
foreach( test_target ${test_programs} )
    get_target_property( test_name ${test_target} OUTPUT_NAME )
    add_test( NAME ${test_name} COMMAND ${test_target} )
    # Generous but finite: the harness watchdog should fire first; this is a backstop.
    set_tests_properties( ${test_name} PROPERTIES TIMEOUT 120 )
endforeach()

# Optional sanitizers -- how latent UB / mismatched alloc bugs get caught.
if( FastDaq_ENABLE_TEST_SANITIZERS )
    foreach( test_target ${test_programs} )
        target_compile_options( ${test_target} PRIVATE -fsanitize=address,undefined -g )
        target_link_options(    ${test_target} PRIVATE -fsanitize=address,undefined )
    endforeach()
endif()

# Benchmarks: built but NOT registered as CTest tests (timing is too noisy for CI).
if( FastDaq_ENABLE_BENCHMARKS )
    set( bench_sources bench_power_averager.cc )
    if( FFTW_FOUND )
        list( APPEND bench_sources bench_frequency_transform.cc )
    endif()

    pbuilder_executables(
        SOURCES ${bench_sources}
        PROJECT_LIBRARIES FastDaqDAQ FastDaqData FastDaqControl FastDaqUtility
        PUBLIC_EXTERNAL_LIBRARIES Catch2::Catch2WithMain
    )
endif()

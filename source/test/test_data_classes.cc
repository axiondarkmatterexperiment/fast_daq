/*
 * test_data_classes.cc
 *
 * Tier 0: unit tests for fast_daq data classes -- no diptera, no threads.
 * Build with -DFastDaq_ENABLE_TEST_SANITIZERS=TRUE to run under ASan/UBSan.
 */

#include <catch2/catch_test_macros.hpp>

#include "frequency_data.hh"
#include "power_data.hh"
#include "real_time_data.hh"
#include "iq_time_data.hh"

using namespace fast_daq;

// ---------------------------------------------------------------------------
// power_data
// ---------------------------------------------------------------------------

TEST_CASE( "power_data default construction", "[data][power_data]" )
{
    power_data d;
    CHECK( d.get_array_size() == 0 );
    CHECK( d.get_data_array() == nullptr );
    CHECK( d.get_bin_width() == 0.f );
    CHECK( d.get_minimum_frequency() == 0.f );
}

TEST_CASE( "power_data allocate_array", "[data][power_data]" )
{
    power_data d;
    d.allocate_array( 16 );
    REQUIRE( d.get_array_size() == 16 );
    REQUIRE( d.get_data_array() != nullptr );

    // Write and read back
    for( unsigned i = 0; i < 16; ++i )
        d.get_data_array()[i] = static_cast< float >( i );
    for( unsigned i = 0; i < 16; ++i )
        CHECK( d.get_data_array()[i] == static_cast< float >( i ) );
}

TEST_CASE( "power_data resize via allocate_array", "[data][power_data]" )
{
    power_data d;
    d.allocate_array( 8 );
    REQUIRE( d.get_array_size() == 8 );

    // A second call should resize (std::vector::resize is idempotent on same size,
    // grows on larger size)
    d.allocate_array( 16 );
    REQUIRE( d.get_array_size() == 16 );
    REQUIRE( d.get_data_array() != nullptr );
}

TEST_CASE( "power_data accessors round-trip", "[data][power_data]" )
{
    power_data d;
    d.set_bin_width( 100.f );
    d.set_minimum_frequency( 1.5e9f );
    CHECK( d.get_bin_width() == 100.f );
    CHECK( d.get_minimum_frequency() == 1.5e9f );
}

// Verify that power_data is not copyable (compile-time check encoded as a type trait)
TEST_CASE( "power_data is not copy-constructible", "[data][power_data]" )
{
    CHECK_FALSE( std::is_copy_constructible< power_data >::value );
    CHECK_FALSE( std::is_copy_assignable< power_data >::value );
}

// ---------------------------------------------------------------------------
// frequency_data
// ---------------------------------------------------------------------------

TEST_CASE( "frequency_data default construction", "[data][frequency_data]" )
{
    frequency_data d;
    CHECK( d.get_array_size() == 0 );
    CHECK( d.get_data_array() == nullptr );
}

TEST_CASE( "frequency_data allocate_array", "[data][frequency_data]" )
{
    frequency_data d;
    d.allocate_array( 8 );
    REQUIRE( d.get_array_size() == 8 );
    REQUIRE( d.get_data_array() != nullptr );

    frequency_data::complex_t* arr = d.get_data_array();
    for( unsigned i = 0; i < 8; ++i )
    {
        arr[i][0] = static_cast< float >( i );
        arr[i][1] = static_cast< float >( i ) + 0.5f;
    }
    for( unsigned i = 0; i < 8; ++i )
    {
        CHECK( arr[i][0] == static_cast< float >( i ) );
        CHECK( arr[i][1] == static_cast< float >( i ) + 0.5f );
    }
}

TEST_CASE( "frequency_data is not copy-constructible", "[data][frequency_data]" )
{
    CHECK_FALSE( std::is_copy_constructible< frequency_data >::value );
    CHECK_FALSE( std::is_copy_assignable< frequency_data >::value );
}

// ---------------------------------------------------------------------------
// real_time_data
// ---------------------------------------------------------------------------

TEST_CASE( "real_time_data default construction", "[data][real_time_data]" )
{
    real_time_data d;
    CHECK( d.get_array_size() == 0 );
    CHECK( d.get_time_series() == nullptr );
    CHECK( d.get_dynamic_range() == 0.f );
}

TEST_CASE( "real_time_data allocate_array", "[data][real_time_data]" )
{
    real_time_data d;
    d.allocate_array( 32 );
    REQUIRE( d.get_array_size() == 32 );
    REQUIRE( d.get_time_series() != nullptr );

    for( unsigned i = 0; i < 32; ++i )
        d.get_time_series()[i] = static_cast< U16 >( i );
    for( unsigned i = 0; i < 32; ++i )
        CHECK( d.get_time_series()[i] == static_cast< U16 >( i ) );
}

TEST_CASE( "real_time_data resize via allocate_array", "[data][real_time_data]" )
{
    real_time_data d;
    d.allocate_array( 16 );
    REQUIRE( d.get_array_size() == 16 );

    d.allocate_array( 32 );
    REQUIRE( d.get_array_size() == 32 );
}

TEST_CASE( "real_time_data is not copy-constructible", "[data][real_time_data]" )
{
    CHECK_FALSE( std::is_copy_constructible< real_time_data >::value );
    CHECK_FALSE( std::is_copy_assignable< real_time_data >::value );
}

// ---------------------------------------------------------------------------
// iq_time_data
// ---------------------------------------------------------------------------

TEST_CASE( "iq_time_data default construction", "[data][iq_time_data]" )
{
    iq_time_data d;
    CHECK( d.get_array_size() == 0 );
    CHECK( d.get_data_array() == nullptr );
}

TEST_CASE( "iq_time_data allocate_container", "[data][iq_time_data]" )
{
    iq_time_data d;
    d.allocate_container( 4 );
    REQUIRE( d.get_array_size() == 4 );
    REQUIRE( d.get_data_array() != nullptr );
}

TEST_CASE( "iq_time_data is not copy-constructible", "[data][iq_time_data]" )
{
    CHECK_FALSE( std::is_copy_constructible< iq_time_data >::value );
    CHECK_FALSE( std::is_copy_assignable< iq_time_data >::value );
}

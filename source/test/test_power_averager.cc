/*
 * test_power_averager.cc
 *
 * Tier 1: correctness tests for power_averager node.
 *
 * Tests:
 *   - Correct power sum over N records of constant (1+0i)
 *   - Correct bin_width and minimum_frequency forwarded to output
 *   - Partial stop (fewer records than num_to_average) flushes with rescaling
 *   - Spectrum-size mismatch throws a node_nonfatal_error (no crash, no hang)
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "midge_test_harness.hh"
#include "midge_test_nodes.hh"

#include "fast_daq_test_data.hh"
#include "power_averager.hh"
#include "frequency_data.hh"
#include "power_data.hh"

using namespace midge::testing;
using namespace fast_daq::testing;

TEST_CASE( "power_averager sums power over N records", "[power_averager]" )
{
    const unsigned t_spectrum_size = 8;
    const unsigned t_n_records     = 4;

    std::vector< captured_power > t_captured;

    auto* t_prod = new test_producer< fast_daq::frequency_data >();
    auto* t_node = new fast_daq::power_averager();
    auto* t_cons = new test_consumer< fast_daq::power_data >();

    t_prod->set_name( "producer" );
    t_node->set_name( "averager" );
    t_cons->set_name( "consumer" );

    t_node->set_spectrum_size( t_spectrum_size );
    t_node->set_num_to_average( t_n_records );

    t_prod->set_n_records( t_n_records );
    t_prod->set_buffer_size( 10 );
    t_prod->set_prepare( [=]( fast_daq::frequency_data* a_slot, std::size_t )
        { a_slot->allocate_array( t_spectrum_size ); } );
    t_prod->set_fill( [=]( fast_daq::frequency_data* a_slot, std::size_t )
        {
            a_slot->set_bin_width( 100.f );
            a_slot->set_minimum_frequency( 1.5e9f );
            fast_daq::frequency_data::complex_t* arr = a_slot->get_data_array();
            for( unsigned i = 0; i < t_spectrum_size; ++i )
            {
                arr[i][0] = 1.f;
                arr[i][1] = 0.f;
            }
        } );

    t_cons->set_expect_records( 1 );
    t_cons->set_extract( [&]( const fast_daq::power_data* a_rec, std::size_t )
        { t_captured.push_back( extract_power( a_rec ) ); } );

    midge::diptera t_midge;
    t_midge.add( t_prod );
    t_midge.add( t_node );
    t_midge.add( t_cons );
    t_midge.join( "producer.out_0:averager.in_0" );
    t_midge.join( "averager.out_0:consumer.in_0" );

    auto t_result = run_with_watchdog( t_midge, "producer:averager:consumer" );

    REQUIRE_FALSE( t_result.f_timed_out );
    REQUIRE_FALSE( t_result.f_exception );

    REQUIRE( t_captured.size() == 1 );
    // |1 + 0i|^2 == 1 per record, x4 records, x (1000/50) mW rescale in the node
    const float t_expected = static_cast< float >( t_n_records ) * ( 1000.f / 50.f );
    for( float v : t_captured[0].f_bins )
        CHECK( v == Catch::Approx( t_expected ).epsilon( 1e-5 ) );
    CHECK( t_captured[0].f_bin_width         == Catch::Approx( 100.f ) );
    CHECK( t_captured[0].f_minimum_frequency == Catch::Approx( 1.5e9f ) );
}

TEST_CASE( "power_averager partial stop flushes remaining records", "[power_averager]" )
{
    // Send fewer records than num_to_average; s_stop should trigger a flush
    const unsigned t_spectrum_size = 4;
    const unsigned t_num_to_avg    = 8;
    const unsigned t_actual_sent   = 3;

    std::vector< captured_power > t_captured;

    auto* t_prod = new test_producer< fast_daq::frequency_data >();
    auto* t_node = new fast_daq::power_averager();
    auto* t_cons = new test_consumer< fast_daq::power_data >();

    t_prod->set_name( "producer" );
    t_node->set_name( "averager" );
    t_cons->set_name( "consumer" );

    t_node->set_spectrum_size( t_spectrum_size );
    t_node->set_num_to_average( t_num_to_avg );

    t_prod->set_n_records( t_actual_sent );
    t_prod->set_buffer_size( 10 );
    t_prod->set_prepare( [=]( fast_daq::frequency_data* a_slot, std::size_t )
        { a_slot->allocate_array( t_spectrum_size ); } );
    t_prod->set_fill( [=]( fast_daq::frequency_data* a_slot, std::size_t )
        {
            a_slot->set_bin_width( 50.f );
            a_slot->set_minimum_frequency( 0.f );
            fast_daq::frequency_data::complex_t* arr = a_slot->get_data_array();
            for( unsigned i = 0; i < t_spectrum_size; ++i )
            {
                arr[i][0] = 2.f;
                arr[i][1] = 0.f;
            }
        } );

    t_cons->set_expect_records( 1 );
    t_cons->set_extract( [&]( const fast_daq::power_data* a_rec, std::size_t )
        { t_captured.push_back( extract_power( a_rec ) ); } );

    midge::diptera t_midge;
    t_midge.add( t_prod );
    t_midge.add( t_node );
    t_midge.add( t_cons );
    t_midge.join( "producer.out_0:averager.in_0" );
    t_midge.join( "averager.out_0:consumer.in_0" );

    auto t_result = run_with_watchdog( t_midge, "producer:averager:consumer" );

    REQUIRE_FALSE( t_result.f_timed_out );
    REQUIRE_FALSE( t_result.f_exception );

    // handle_stop should have flushed the partial sum
    REQUIRE( t_captured.size() == 1 );
    // |2 + 0i|^2 == 4 per record, x3 records, rescaled back to num_to_avg=8,
    // then x (1000/50) mW.  The node rescales by max(num_to_avg,1)/input_counter
    // when a partial flush occurs.
    const float t_raw_sum   = 4.f * static_cast< float >( t_actual_sent );
    const float t_rescale   = static_cast< float >( t_num_to_avg ) / static_cast< float >( t_actual_sent );
    const float t_expected  = t_raw_sum * t_rescale * ( 1000.f / 50.f );
    for( float v : t_captured[0].f_bins )
        CHECK( v == Catch::Approx( t_expected ).epsilon( 1e-5 ) );
}

TEST_CASE( "power_averager rejects spectrum-size mismatch", "[power_averager]" )
{
    const unsigned t_configured_size = 8;
    const unsigned t_actual_size     = 16;  // larger than configured

    auto* t_prod = new test_producer< fast_daq::frequency_data >();
    auto* t_node = new fast_daq::power_averager();
    auto* t_cons = new test_consumer< fast_daq::power_data >();

    t_prod->set_name( "producer" );
    t_node->set_name( "averager" );
    t_cons->set_name( "consumer" );

    t_node->set_spectrum_size( t_configured_size );
    t_node->set_num_to_average( 1 );

    t_prod->set_n_records( 1 );
    t_prod->set_buffer_size( 10 );
    t_prod->set_prepare( [=]( fast_daq::frequency_data* a_slot, std::size_t )
        { a_slot->allocate_array( t_actual_size ); } );
    t_prod->set_fill( [=]( fast_daq::frequency_data* a_slot, std::size_t )
        {
            fast_daq::frequency_data::complex_t* arr = a_slot->get_data_array();
            for( unsigned i = 0; i < t_actual_size; ++i )
            {
                arr[i][0] = 1.f;
                arr[i][1] = 0.f;
            }
        } );

    t_cons->set_expect_records( 0 );  // expect no output -- the node should throw

    midge::diptera t_midge;
    t_midge.add( t_prod );
    t_midge.add( t_node );
    t_midge.add( t_cons );
    t_midge.join( "producer.out_0:averager.in_0" );
    t_midge.join( "averager.out_0:consumer.in_0" );

    auto t_result = run_with_watchdog( t_midge, "producer:averager:consumer" );

    // The node should throw, not hang or abort the process
    REQUIRE_FALSE( t_result.f_timed_out );
    // diptera captures the exception via throw_ex; f_exception should be non-null
    REQUIRE( t_result.f_exception );
}

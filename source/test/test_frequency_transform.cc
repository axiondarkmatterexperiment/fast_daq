/*
 * test_frequency_transform.cc
 *
 * Tier 1: correctness test for frequency_transform node.
 * Gated on FFTW_FOUND (set by the top-level CMakeLists.txt).
 *
 * frequency_transform has two input slots (0: time_data/IQ, 1: real_time_data)
 * selected by input_type. This test uses input_type == "real" (slot 1).
 *
 * frequency_transform is one of the few nodes that correctly handles s_exit
 * and breaks its own loop, so it does not require a cancel-based shutdown.
 * The test_consumer will still cancel after receiving one output record to
 * keep the test short.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "midge_test_harness.hh"
#include "midge_test_nodes.hh"

#include "fast_daq_test_data.hh"
#include "frequency_transform.hh"
#include "real_time_data.hh"
#include "frequency_data.hh"

using namespace midge::testing;
using namespace fast_daq::testing;

TEST_CASE( "frequency_transform produces frequency_data from real_time_data", "[frequency_transform]" )
{
    const unsigned t_fft_size     = 64;
    const unsigned t_data_size    = t_fft_size;
    const unsigned t_samples_sec  = 200000000;  // 200 MHz
    const unsigned t_freq_length  = 5;

    std::vector< captured_frequency > t_captured;

    auto* t_prod = new test_producer< fast_daq::real_time_data >();
    auto* t_node = new fast_daq::frequency_transform();
    auto* t_cons = new test_consumer< fast_daq::frequency_data >();

    t_prod->set_name( "producer" );
    t_node->set_name( "transform" );
    t_cons->set_name( "consumer" );

    t_node->set_input_type( std::string( "real" ) );
    t_node->set_fft_size( t_fft_size );
    t_node->set_samples_per_sec( t_samples_sec );
    t_node->set_freq_length( t_freq_length );
    t_node->set_use_wisdom( false );

    t_prod->set_n_records( 2 );
    t_prod->set_buffer_size( 8 );
    t_prod->set_prepare( [=]( fast_daq::real_time_data* a_slot, std::size_t )
        { a_slot->allocate_array( t_data_size ); } );
    // DC offset signal: all samples = 32768 (midscale for U16)
    t_prod->set_fill( [=]( fast_daq::real_time_data* a_slot, std::size_t )
        {
            for( unsigned i = 0; i < t_data_size; ++i )
                a_slot->get_time_series()[i] = 32768;
        } );

    t_cons->set_expect_records( 1 );
    t_cons->set_extract( [&]( const fast_daq::frequency_data* a_rec, std::size_t )
        { t_captured.push_back( extract_frequency( a_rec ) ); } );

    midge::diptera t_midge;
    t_midge.add( t_prod );
    t_midge.add( t_node );
    t_midge.add( t_cons );
    // input_type == "real" uses slot 1 (in_1)
    t_midge.join( "producer.out_0:transform.in_1" );
    t_midge.join( "transform.out_0:consumer.in_0" );

    auto t_result = run_with_watchdog( t_midge, "producer:transform:consumer" );

    REQUIRE_FALSE( t_result.f_timed_out );
    REQUIRE_FALSE( t_result.f_exception );

    REQUIRE( t_captured.size() >= 1 );

    // Basic structural checks: output should have bins, non-zero bin width
    const auto& rec = t_captured[0];
    REQUIRE( rec.f_bins.size() > 0 );
    CHECK( rec.f_bin_width > 0.f );
}

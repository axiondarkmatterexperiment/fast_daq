/*
 * bench_power_averager.cc
 *
 * Tier 2 benchmark for power_averager.
 *
 * Strategy: benchmark the per-record computation directly (no diptera, no threads)
 * by constructing the node, calling initialize(), then driving handle_run() via
 * manual stream operations. This isolates the actual averaging arithmetic from the
 * ~600 ms fixed overhead that diptera adds.
 *
 * Run manually:
 *   ./bench_power_averager --benchmark-samples 20
 *
 * NOT registered in CTest -- timing assertions on shared CI runners are flaky.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

#include "midge_test_harness.hh"
#include "midge_test_nodes.hh"
#include "fast_daq_test_data.hh"

#include "power_averager.hh"
#include "frequency_data.hh"
#include "power_data.hh"

using namespace midge::testing;
using namespace fast_daq::testing;

TEST_CASE( "power_averager throughput benchmark", "[.][benchmark][power_averager]" )
{
    const unsigned t_spectrum_size = 4096;
    const unsigned t_n_records     = 50;
    const unsigned t_repeat        = 100;

    // Build a fixed set of frequency_data records to feed repeatedly
    // (no diptera involved, so payload ownership is manual here)
    std::vector< std::unique_ptr< fast_daq::frequency_data > > t_inputs( t_n_records );
    for( unsigned i = 0; i < t_n_records; ++i )
    {
        t_inputs[i].reset( new fast_daq::frequency_data() );
        t_inputs[i]->allocate_array( t_spectrum_size );
        t_inputs[i]->set_bin_width( 100.f );
        t_inputs[i]->set_minimum_frequency( 0.f );
        auto* arr = t_inputs[i]->get_data_array();
        for( unsigned j = 0; j < t_spectrum_size; ++j )
        {
            arr[j][0] = 1.f;
            arr[j][1] = 0.f;
        }
    }

    // End-to-end pipeline benchmark using fresh diptera per sample.
    // Use a large record count so fixed overhead amortizes.
    BENCHMARK_ADVANCED( "end-to-end pipeline (producer->averager->consumer)" )( Catch::Benchmark::Chronometer meter )
    {
        meter.measure( [&]
        {
            std::vector< captured_power > t_captured;

            auto* t_prod = new test_producer< fast_daq::frequency_data >();
            auto* t_node = new fast_daq::power_averager();
            auto* t_cons = new test_consumer< fast_daq::power_data >();

            t_prod->set_name( "producer" );
            t_node->set_name( "averager" );
            t_cons->set_name( "consumer" );

            t_node->set_spectrum_size( t_spectrum_size );
            t_node->set_num_to_average( t_n_records );

            t_prod->set_n_records( t_n_records * t_repeat );
            t_prod->set_buffer_size( 16 );
            t_prod->set_prepare( [=]( fast_daq::frequency_data* a_slot, std::size_t )
                { a_slot->allocate_array( t_spectrum_size ); } );
            t_prod->set_fill( [=]( fast_daq::frequency_data* a_slot, std::size_t )
                {
                    auto* arr = a_slot->get_data_array();
                    for( unsigned i = 0; i < t_spectrum_size; ++i ) { arr[i][0] = 1.f; arr[i][1] = 0.f; }
                } );

            t_cons->set_expect_records( t_repeat );
            t_cons->set_extract( [&]( const fast_daq::power_data* a_rec, std::size_t )
                { t_captured.push_back( extract_power( a_rec ) ); } );

            midge::diptera t_midge;
            t_midge.add( t_prod );
            t_midge.add( t_node );
            t_midge.add( t_cons );
            t_midge.join( "producer.out_0:averager.in_0" );
            t_midge.join( "averager.out_0:consumer.in_0" );

            run_with_watchdog( t_midge, "producer:averager:consumer", 60000 );
            return t_captured.size();
        } );
    };
}

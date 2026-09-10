/*
 * bench_frequency_transform.cc
 *
 * Tier 2 benchmark for frequency_transform.
 * Gated on FFTW_FOUND.
 *
 * Run manually:
 *   ./bench_frequency_transform --benchmark-samples 20
 *
 * NOT registered in CTest.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

#include "midge_test_harness.hh"
#include "midge_test_nodes.hh"
#include "fast_daq_test_data.hh"

#include "frequency_transform.hh"
#include "real_time_data.hh"
#include "frequency_data.hh"

using namespace midge::testing;
using namespace fast_daq::testing;

TEST_CASE( "frequency_transform throughput benchmark", "[.][benchmark][frequency_transform]" )
{
    const unsigned t_fft_size    = 4096;
    const unsigned t_data_size   = t_fft_size;
    const unsigned t_samples_sec = 200000000;
    const unsigned t_n_records   = 200;
    const unsigned t_freq_length = 5;

    BENCHMARK_ADVANCED( "end-to-end pipeline (producer->transform->consumer)" )( Catch::Benchmark::Chronometer meter )
    {
        meter.measure( [&]
        {
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

            t_prod->set_n_records( t_n_records );
            t_prod->set_buffer_size( 8 );
            t_prod->set_prepare( [=]( fast_daq::real_time_data* a_slot, std::size_t )
                { a_slot->allocate_array( t_data_size ); } );
            t_prod->set_fill( [=]( fast_daq::real_time_data* a_slot, std::size_t )
                {
                    for( unsigned i = 0; i < t_data_size; ++i )
                        a_slot->get_time_series()[i] = 32768;
                } );

            t_cons->set_expect_records( t_n_records );
            t_cons->set_extract( [&]( const fast_daq::frequency_data* a_rec, std::size_t )
                { t_captured.push_back( extract_frequency( a_rec ) ); } );

            midge::diptera t_midge;
            t_midge.add( t_prod );
            t_midge.add( t_node );
            t_midge.add( t_cons );
            t_midge.join( "producer.out_0:transform.in_1" );
            t_midge.join( "transform.out_0:consumer.in_0" );

            run_with_watchdog( t_midge, "producer:transform:consumer", 60000 );
            return t_captured.size();
        } );
    };
}

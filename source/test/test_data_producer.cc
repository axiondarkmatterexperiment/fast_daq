/*
 * test_data_producer.cc
 *
 * Tier 1: correctness test for data_producer node.
 * data_producer runs forever (loops on is_canceled()), so the test_consumer
 * cancels the run after receiving a configurable number of records.
 *
 * Note: data_producer has a configurable delay_time_ms (default 500). Always
 * set it to 0 in tests -- otherwise each record adds 500 ms latency.
 */

#include <catch2/catch_test_macros.hpp>

#include "midge_test_harness.hh"
#include "midge_test_nodes.hh"

#include "fast_daq_test_data.hh"
#include "data_producer.hh"
#include "real_time_data.hh"

using namespace midge::testing;
using namespace fast_daq::testing;

TEST_CASE( "data_producer emits real_time_data records", "[data_producer]" )
{
    const unsigned t_data_size = 64;
    const unsigned t_data_value = 42;
    const std::size_t t_want = 3;

    std::vector< captured_real_time > t_captured;

    // data_producer runs forever: test_consumer drives shutdown via cancel
    auto* t_prod = new fast_daq::data_producer();
    auto* t_cons = new test_consumer< fast_daq::real_time_data >();

    t_prod->set_name( "producer" );
    t_cons->set_name( "consumer" );

    t_prod->set_data_size( t_data_size );
    t_prod->set_data_value( static_cast< uint16_t >( t_data_value ) );
    t_prod->set_delay_time_ms( 0 );  // must be 0 -- default is 500 ms

    t_cons->set_expect_records( t_want );
    t_cons->set_extract( [&]( const fast_daq::real_time_data* a_rec, std::size_t )
        { t_captured.push_back( extract_real_time( a_rec ) ); } );

    midge::diptera t_midge;
    t_midge.add( t_prod );
    t_midge.add( t_cons );
    t_midge.join( "producer.out_0:consumer.in_0" );

    auto t_result = run_with_watchdog( t_midge, "producer:consumer" );

    REQUIRE_FALSE( t_result.f_timed_out );
    REQUIRE_FALSE( t_result.f_exception );

    // We asked for t_want records; we may receive more because the producer
    // races with cancellation.
    REQUIRE( t_captured.size() >= t_want );

    // Every record should have the configured size and value
    for( const auto& rec : t_captured )
    {
        REQUIRE( rec.f_samples.size() == t_data_size );
        for( U16 v : rec.f_samples )
            CHECK( v == static_cast< U16 >( t_data_value ) );
    }
}

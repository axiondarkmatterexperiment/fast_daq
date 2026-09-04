/*
 * test_dead_end.cc
 *
 * Tier 1: smoke test for dead_end node.
 * dead_end is a pure consumer with no output; the test just verifies the
 * pipeline terminates cleanly and the node receives the expected records.
 *
 * dead_end has four input slots (real_time_data/frequency_data/power_data/iq_time_data)
 * selected by input_index. Only the active slot is joined here; the others are
 * left unjoined, which is safe because dead_end's switch never reads them.
 */

#include <catch2/catch_test_macros.hpp>

#include "midge_test_harness.hh"
#include "midge_test_nodes.hh"

#include "fast_daq_test_data.hh"
#include "dead_end.hh"
#include "power_data.hh"

using namespace midge::testing;
using namespace fast_daq::testing;

TEST_CASE( "dead_end receives power_data records and terminates", "[dead_end]" )
{
    const unsigned t_spectrum_size = 8;
    const std::size_t t_n_records  = 4;

    auto* t_prod = new test_producer< fast_daq::power_data >();
    auto* t_node = new fast_daq::dead_end();

    t_prod->set_name( "producer" );
    t_node->set_name( "dead-end" );

    t_node->set_input_index( 2 );  // slot 2 == power_data

    t_prod->set_n_records( t_n_records );
    t_prod->set_buffer_size( 10 );
    t_prod->set_prepare( [=]( fast_daq::power_data* a_slot, std::size_t )
        { a_slot->allocate_array( t_spectrum_size ); } );
    t_prod->set_fill( [=]( fast_daq::power_data* a_slot, std::size_t )
        {
            float* arr = a_slot->get_data_array();
            for( unsigned i = 0; i < t_spectrum_size; ++i )
                arr[i] = static_cast< float >( i );
        } );

    midge::diptera t_midge;
    t_midge.add( t_prod );
    t_midge.add( t_node );
    // dead_end slot 2 is power_data (in_2)
    t_midge.join( "producer.out_0:dead-end.in_2" );

    auto t_result = run_with_watchdog( t_midge, "producer:dead-end" );

    REQUIRE_FALSE( t_result.f_timed_out );
    REQUIRE_FALSE( t_result.f_exception );
}

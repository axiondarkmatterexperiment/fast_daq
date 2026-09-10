/*
 * test_power_averager_binding.cc
 *
 * Tests that power_averager_binding correctly round-trips its configuration:
 * every key that do_apply_config reads must appear in do_dump_config's output.
 */

#include <catch2/catch_test_macros.hpp>

#include "sandfly_test_binding.hh"

#include "power_averager.hh"

using namespace sandfly::testing;
using namespace fast_daq;

TEST_CASE( "power_averager_binding round-trips num-output-buffers", "[binding][power_averager]" )
{
    power_averager t_node;
    power_averager_binding t_binding;

    scarab::param_node t_config;
    t_config.add( "num-output-buffers", scarab::param_value( 3u ) );

    auto t_dumped = config_round_trip( &t_node, t_binding, t_config );

    CHECK( t_node.get_num_output_buffers() == 3u );
    REQUIRE( t_dumped.has( "num-output-buffers" ) );
    CHECK( t_dumped["num-output-buffers"]().as_uint() == 3u );
}

TEST_CASE( "power_averager_binding round-trips spectrum-size", "[binding][power_averager]" )
{
    power_averager t_node;
    power_averager_binding t_binding;

    scarab::param_node t_config;
    t_config.add( "spectrum-size", scarab::param_value( 1024u ) );

    auto t_dumped = config_round_trip( &t_node, t_binding, t_config );

    CHECK( t_node.get_spectrum_size() == 1024u );
    REQUIRE( t_dumped.has( "spectrum-size" ) );
    CHECK( t_dumped["spectrum-size"]().as_uint() == 1024u );
}

TEST_CASE( "power_averager_binding round-trips num-to-average", "[binding][power_averager]" )
{
    power_averager t_node;
    power_averager_binding t_binding;

    scarab::param_node t_config;
    t_config.add( "num-to-average", scarab::param_value( 10u ) );

    auto t_dumped = config_round_trip( &t_node, t_binding, t_config );

    CHECK( t_node.get_num_to_average() == 10u );
    REQUIRE( t_dumped.has( "num-to-average" ) );
    CHECK( t_dumped["num-to-average"]().as_uint() == 10u );
}

TEST_CASE( "power_averager_binding preserves defaults for unset keys", "[binding][power_averager]" )
{
    power_averager t_node;
    power_averager_binding t_binding;

    // Apply an empty config -- node should keep its constructor defaults
    auto t_dumped = config_round_trip( &t_node, t_binding, scarab::param_node() );

    REQUIRE( t_dumped.has( "num-output-buffers" ) );
    REQUIRE( t_dumped.has( "spectrum-size" ) );
    REQUIRE( t_dumped.has( "num-to-average" ) );
}

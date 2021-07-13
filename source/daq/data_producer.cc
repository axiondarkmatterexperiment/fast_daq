/*
 * data_producer.cc
 *
 *  Created on: May 31, 2019
 *      Author: obla999
 */

#include "data_producer.hh"

#include "logger.hh"

#include <thread>
#include <chrono>

LOGGER( plog, "data_producer" );

using midge::stream;

namespace fast_daq
{

    REGISTER_NODE_AND_BUILDER( data_producer, "data-producer", data_producer_binding );

    data_producer::data_producer() :
            f_length( 10 ),
            f_data_size( 16384 ),
            f_data_value( 5 ),
            f_dynamic_range( 1. ),
            f_delay_time_ms( 500 ),
            f_primary_packet()
    {
        write_primary_packet();
    }

    data_producer::~data_producer()
    {
    }

    void data_producer::write_primary_packet()
    {
        f_primary_packet.allocate_array( f_data_size );
        f_primary_packet.set_dynamic_range( f_dynamic_range );
        for( unsigned i_bin = 0; i_bin < f_data_size; ++i_bin )
        {
            f_primary_packet.get_time_series()[i_bin] = f_data_value;
        }
        return;
    }

    void data_producer::initialize()
    {
        out_buffer< 0 >().initialize( f_length );
    }

    void data_producer::execute( midge::diptera* a_midge )
    {
        LDEBUG( plog, "Executing the data_producer" );

        try
        {
            real_time_data* t_block = nullptr;

            //LDEBUG( plog, "Server is listening" );

            if( ! out_stream< 0 >().set( stream::s_start ) ) return;

            ssize_t t_size_received = 0;

            LINFO( plog, "Starting main loop; sending packets" );
            //unsigned count = 0;
            while( ! is_canceled() )
            {
                t_block = out_stream< 0 >().data();
                if( t_block->get_array_size() != f_data_size )
                {
                    initialize_block( t_block );
                }

                t_size_received = 0;

                if( out_stream< 0 >().get() == stream::s_stop )
                {
                    LWARN( plog, "Output stream(s) have stop condition" );
                    break;
                }

                if( ! out_stream< 0 >().set( stream::s_run ) )
                {
                    LERROR( plog, "Exiting due to stream error" );
                    break;
                }

                std::this_thread::sleep_for( std::chrono::milliseconds(f_delay_time_ms) );
            }

            LINFO( plog, "Data producer is exiting" );

            // normal exit condition
            LDEBUG( plog, "Stopping output stream" );
            if( ! out_stream< 0 >().set( stream::s_stop ) ) return;

            LDEBUG( plog, "Exiting output stream" );
            out_stream< 0 >().set( stream::s_exit );

            return;
        }
        catch(...)
        {
            a_midge->throw_ex( std::current_exception() );
        }
    }

    void data_producer::finalize()
    {
        out_buffer< 0 >().finalize();

        return;
    }

    void data_producer::initialize_block( real_time_data* a_block )
    {
        a_block->allocate_array( f_data_size );

        ::memcpy( a_block->get_time_series(), f_primary_packet.get_time_series(), f_data_size*sizeof(U16) );

        return;
    }

    data_producer_binding::data_producer_binding() :
            sandfly::_node_binding< data_producer, data_producer_binding >()
    {
    }

    data_producer_binding::~data_producer_binding()
    {
    }

    void data_producer_binding::do_apply_config( data_producer* a_node, const scarab::param_node& a_config ) const
    {
        LDEBUG( plog, "Configuring data_producer with:\n" << a_config );
        a_node->set_length( a_config.get_value( "length", a_node->get_length() ) );
        a_node->set_data_size( a_config.get_value( "data-size", a_node->get_data_size() ) );
        a_node->set_data_value( a_config.get_value( "data-value", a_node->get_data_value() ) );
        a_node->set_dynamic_range( a_config.get_value( "dynamic-range", a_node->get_dynamic_range() ) );
        a_node->set_delay_time_ms( a_config.get_value( "delay-time-ms", a_node->get_delay_time_ms() ) );
        return;
    }

    void data_producer_binding::do_dump_config( const data_producer* a_node, scarab::param_node& a_config ) const
    {
        LDEBUG( plog, "Dumping data_producer configuration" );
        a_config.add( "length", scarab::param_value( a_node->get_length() ) );
        a_config.add( "data-size", scarab::param_value( a_node->get_data_size() ) );
        a_config.add( "data-value", scarab::param_value( a_node->get_data_value() ) );
        a_config.add( "dynamic-range", scarab::param_value( a_node->get_dynamic_range() ) );
        a_config.add( "delay-time-ms", scarab::param_value( a_node->get_delay_time_ms() ) );
        return;

    }

} /* namespace fast_daq */

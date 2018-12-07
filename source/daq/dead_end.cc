/*
 * dead_end.cc
 *
 * Created on: Nov. 28, 2018
 *     Author: laroque
 */

#include <stdio.h>

#include "logger.hh"

//#include "daq_control.hh"
#include "real_time_data.hh"
#include "dead_end.hh"


using midge::stream;

namespace fast_daq
{
    REGISTER_NODE_AND_BUILDER( dead_end, "dead-end", dead_end_binding );

    LOGGER( flog, "dead_end" );

    /* dead_end class */
    /***************************/

    // dead_end methods
    dead_end::dead_end() //:
    {
    }

    dead_end::~dead_end()
    {
    }

    // node interface methods
    void dead_end::initialize()
    {
    }

    void dead_end::execute( midge::diptera* a_midge )
    {
        try
        {
            while (! is_canceled() )
            {
                // Check for midge instructions
                if( have_instruction() )
                {
                    LDEBUG( flog, "WARNING: dead_end does not support any instructions");
                }
                // check the slot status
                midge::enum_t input_command = stream::s_none;
                input_command = in_stream< 0 >().get();
                if ( input_command == midge::stream::s_none )
                {
                    continue;
                }
                else if ( input_command == stream::s_error )
                {
                    LDEBUG( flog, " got an s_error on slot <" << in_stream< 0 >().get_current_index() << "> of stream <" << 0 << ">");
                }
                else if ( input_command == stream::s_stop )
                {
                    LDEBUG( flog, " got an s_stop on slot <" << in_stream< 0 >().get_current_index() << "> of stream <" << 0 << ">");
                    continue;
                }
                else if ( input_command == stream::s_start )
                {
                    LDEBUG( flog, " got an s_start on slot <" << in_stream< 0 >().get_current_index() << "> of stream <" << 0 << ">");
                    continue;
                }
                else if ( input_command == stream::s_run )
                {
                    LDEBUG( flog, " got an s_run on slot <" << in_stream< 0 >().get_current_index() << "> of stream <" << 0 << ">");
                    continue;
                }
            }
        }
        catch( std::exception )
        {
            a_midge->throw_ex( std::current_exception() );
        }
    }

    void dead_end::finalize()
    {
    }

    /* dead_end_binding class */
    /***********************************/
    // dead_end_binding methods
    dead_end_binding::dead_end_binding()
    {
    }

    dead_end_binding::~dead_end_binding()
    {
    }

    void dead_end_binding::do_apply_config(dead_end* a_node, const scarab::param_node& a_config ) const
    {
        //a_node->set_samples_per_buffer( a_config.get_value( "samples-per-buffer", a_node->get_samples_per_buffer() ) );
    }

    void dead_end_binding::do_dump_config( const dead_end* a_node, scarab::param_node& a_config ) const
    {
        //a_config.add( "samples-per-bufer", scarab::param_value( a_node->get_samples_per_buffer() ) );
    }

} /* namespace fast_daq */

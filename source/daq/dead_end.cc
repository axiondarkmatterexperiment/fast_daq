/*
 * dead_end.cc
 *
 * Created on: Nov. 28, 2018
 *     Author: laroque
 */

#include <stdio.h>

//scarab includes
#include "logger.hh"

//fast_daq includes
#include "dead_end.hh"
#include "frequency_data.hh"
#include "iq_time_data.hh"
#include "real_time_data.hh"
#include "power_data.hh"


using midge::stream;

namespace fast_daq
{
    REGISTER_NODE_AND_BUILDER( dead_end, "dead-end", dead_end_binding );

    LOGGER( flog, "dead_end" );

    /* dead_end class */
    /***************************/

    // dead_end methods
    dead_end::dead_end() :
        f_input_index( 0 )
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
            unsigned t_received = 0;
            while (! is_canceled() )
            {
                // Check for midge instructions
                if( have_instruction() )
                {
                    //LDEBUG( flog, "WARNING: dead_end does not support any instructions");
                }

                // check the slot status
                midge::enum_t input_command = stream::s_none;
                unsigned stream_index = 0;
                unsigned stream_id = 0;
                switch (f_input_index)
                {
                    case 0:
                        input_command = in_stream< 0 >().get();
                        stream_index = in_stream< 0 >().get_current_index();
                        stream_id = 0;
                        break;
                    case 1:
                        input_command = in_stream< 1 >().get();
                        stream_index = in_stream< 1 >().get_current_index();
                        stream_id = 1;
                        break;
                    case 2:
                        input_command = in_stream< 2 >().get();
                        stream_index = in_stream< 2 >().get_current_index();
                        stream_id = 2;
                        break;
                    case 3:
                        input_command = in_stream< 3 >().get();
                        stream_index = in_stream< 3 >().get_current_index();
                        stream_id = 3;
                        break;
                    default:
                        LERROR( flog, "input type not recognized!" );
                        throw sandfly::error() << "input index <" << f_input_index << "> not recognized";
                }
                if ( input_command == midge::stream::s_none )
                {
                    continue;
                }
                else if ( input_command == stream::s_error )
                {
                    LWARN( flog, " got an s_error on slot <" << stream_index << "> of stream <" << stream_id << ">");
                }
                else if ( input_command == stream::s_stop )
                {
                    LINFO( flog, " got an s_stop on slot <" << stream_index << "> of stream <" << stream_id << ">");
                    LDEBUG( flog, "received a total of [" << t_received << "] stream< " << stream_id << " > values" );
                    continue;
                }
                else if ( input_command == stream::s_start )
                {
                    LDEBUG( flog, " got an s_start on slot <" << stream_index << "> of stream <" << stream_id << ">");
                    t_received = 0;
                    continue;
                }
                else if ( input_command == stream::s_run )
                {
                    LTRACE( flog, " got an s_run on slot <" << stream_index << "> of stream <" << stream_id << ">");
                    t_received++;
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
        a_node->set_input_index( a_config.get_value( "input-index", a_node->get_input_index() ) );
    }

    void dead_end_binding::do_dump_config( const dead_end* a_node, scarab::param_node& a_config ) const
    {
        a_config.add( "input-index", scarab::param_value( a_node->get_input_index() ) );
    }

} /* namespace fast_daq */

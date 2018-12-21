/*
 * spectrum_relay.cc
 *
 * Created on: Dec. 18, 2018
 *     Author: laroque
 */

#include <stdio.h>

//scarab includes
#include "logger.hh"

//psyllid
#include "message_relayer.hh"

//fast_daq includes
#include "spectrum_relay.hh"
#include "power_data.hh"


using midge::stream;

namespace fast_daq
{
    REGISTER_NODE_AND_BUILDER( spectrum_relay, "spectrum-relay", spectrum_relay_binding );

    LOGGER( flog, "spectrum_relay" );

    /* spectrum_relay class */
    /***************************/

    // spectrum_relay methods
    spectrum_relay::spectrum_relay() :
        f_msg_relay( psyllid::message_relayer::get_instance() )
    {
    }

    spectrum_relay::~spectrum_relay()
    {
    }

    // node interface methods
    void spectrum_relay::initialize()
    {
    }

    void spectrum_relay::execute( midge::diptera* a_midge )
    {
        try
        {
            while (! is_canceled() )
            {
                // Check for midge instructions
                if( have_instruction() )
                {
                    //LDEBUG( flog, "WARNING: spectrum_relay does not support any instructions");
                }

                // check the slot status
                midge::enum_t input_command = in_stream< 0 >().get();
                unsigned stream_index = in_stream< 0 >().get_current_index();
                if ( input_command == midge::stream::s_none )
                {
                    continue;
                }
                else if ( input_command == stream::s_error )
                {
                    LWARN( flog, " got an s_error on slot <" << stream_index << ">");
                }
                else if ( input_command == stream::s_stop )
                {
                    LWARN( flog, " got an s_stop on slot <" << stream_index << ">");
                    continue;
                }
                else if ( input_command == stream::s_start )
                {
                    LDEBUG( flog, " got an s_start on slot <" << stream_index << ">");
                    continue;
                }
                else if ( input_command == stream::s_run )
                {
                    LTRACE( flog, " got an s_run on slot <" << stream_index << ">");
                    LWARN( flog, "got some data, that's nice" );
                    //power_data* data_in = in_stream< 0 >().data();
                    f_msg_relay->slack_warn( std::string("got a spectrum" ) );
                    continue;
                }
            }
        }
        catch( std::exception )
        {
            a_midge->throw_ex( std::current_exception() );
        }
    }

    void spectrum_relay::finalize()
    {
    }

    /* spectrum_relay_binding class */
    /***********************************/
    // spectrum_relay_binding methods
    spectrum_relay_binding::spectrum_relay_binding()
    {
    }

    spectrum_relay_binding::~spectrum_relay_binding()
    {
    }

    void spectrum_relay_binding::do_apply_config(spectrum_relay* /* a_node */, const scarab::param_node& /* a_config */ ) const
    {
        //a_node->set_input_index( a_config.get_value( "input-index", a_node->get_input_index() ) );
    }

    void spectrum_relay_binding::do_dump_config( const spectrum_relay* /* a_node */, scarab::param_node& /* a_config */ ) const
    {
        //a_config.add( "input-index", scarab::param_value( a_node->get_input_index() ) );
    }

} /* namespace fast_daq */

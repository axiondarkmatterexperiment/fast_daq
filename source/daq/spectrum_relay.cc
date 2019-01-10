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
#include "global_config.hh"
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
        f_dl_relay( psyllid::global_config::get_instance()->retrieve()["amqp"].as_node() )
    {
    }

    spectrum_relay::~spectrum_relay()
    {
    }

    // node interface methods
    void spectrum_relay::initialize()
    {
        f_dl_relay_thread = std::thread( &dripline::relayer::execute_relayer, &f_dl_relay );
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
                    //TODO clean this
                    LTRACE( flog, " got an s_run on slot <" << stream_index << ">");
                    LERROR( flog, "got some data, that's nice" );
                    power_data* data_in = in_stream< 0 >().data();
                    scarab::param_node t_payload = scarab::param_node();
                    t_payload.add("value_raw", scarab::param_array());
                    //TODO if the data were in an std::vector, I could std::for_each instead of this
                    for (unsigned i_bin=0; i_bin<data_in->get_array_size(); i_bin++)
                    {
                        t_payload["value_raw"].as_array().push_back( data_in->get_data_array()[i_bin] );
                    }
                    send_alert_message("data.target", t_payload);
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
        f_dl_relay.cancel();
        if ( f_dl_relay_thread.joinable() )
        {
            f_dl_relay_thread.join();
        }
    }

    void spectrum_relay::send_alert_message( std::string a_routing_key, scarab::param_node a_payload )
    {
        //scarab::param_node t_msg = scarab::param_node();
        //t_msg.add( "message", scarab::param_value( "something to say and send" ) );
        f_dl_relay.send_async( dripline::msg_alert::create( a_payload, "data.from.psyllid" ) );
        LINFO( flog, "sent an alert with rk <" << a_routing_key << ">" );
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

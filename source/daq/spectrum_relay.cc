/*
 * spectrum_relay.cc
 *
 * Created on: Dec. 18, 2018
 *     Author: laroque
 */

#include <stdio.h>

//scarab includes
#include "logger.hh"
#include "param_json.hh"

//sandfly
#include "daq_control.hh"
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
        f_spectrum_alert_rk( "spectrum-data" )
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
                    LINFO( flog, " got an s_stop on slot <" << stream_index << ">");
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
                    power_data* data_in = in_stream< 0 >().data();
                    broadcast_spectrum( data_in );
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

    void spectrum_relay::broadcast_spectrum( power_data* a_spectrum )
    {
        // grab the run description and load it into the broadcast payload
        scarab::param_ptr_t t_payload_ptr( new scarab::param_node() );
        scarab::param_node& t_payload = t_payload_ptr->as_node();
        std::shared_ptr< sandfly::run_control > t_daq_control = use_daq_control();
        scarab::param_input_json t_param_codec;
        t_payload.merge( t_param_codec.read_string( t_daq_control->get_description() )->as_node() );
        scarab::param_array t_spectrum_array;
        for (unsigned i_bin=0; i_bin < a_spectrum->get_array_size(); ++i_bin)
        {
            t_spectrum_array.push_back( a_spectrum->get_data_array()[i_bin] );
        }
        t_payload.add( "value_raw", std::move( t_spectrum_array) );
        t_payload.add( "minimum_frequency", a_spectrum->get_minimum_frequency() );
        t_payload.add( "maximum_frequency", a_spectrum->get_minimum_frequency() + a_spectrum->get_array_size() * a_spectrum->get_bin_width() );
        t_payload.add( "frequency_resolution", a_spectrum->get_bin_width() );
        // send it
        ::message_relayer* t_message_relay = sandfly::message_relayer::get_instance();
        t_message_relay->send( dripline::msg_alert::create( std::move(t_payload_ptr), f_spectrum_alert_rk ) );
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

    void spectrum_relay_binding::do_apply_config(spectrum_relay* a_node, const scarab::param_node& a_config ) const
    {
        a_node->set_spectrum_alert_rk( a_config.get_value( "spectrum-alert-rk", a_node->get_spectrum_alert_rk() ) );
    }

    void spectrum_relay_binding::do_dump_config( const spectrum_relay* a_node, scarab::param_node& a_config ) const
    {
        a_config.add( "spectrum-alert-rk", scarab::param_value( a_node->get_spectrum_alert_rk() ) );
    }

} /* namespace fast_daq */

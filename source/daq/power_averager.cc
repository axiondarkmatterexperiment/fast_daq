/*
 * power_averager.cc
 *
 * Created on: Nov. 17, 2018
 *     Author: laroque
 */

#include <stdio.h>

//scarab includes
#include "logger.hh"

//fast_daq includes
#include "power_averager.hh"
#include "frequency_data.hh"
#include "real_time_data.hh"


using midge::stream;

namespace fast_daq
{
    REGISTER_NODE_AND_BUILDER( power_averager, "dead-end", power_averager_binding );

    LOGGER( flog, "power_averager" );

    /* power_averager class */
    /***************************/

    // power_averager methods
    power_averager::power_averager() :
        f_num_output_buffers( 1 ),
        f_spectrum_size(),
        f_accumulator_array()
    {
    }

    power_averager::~power_averager()
    {
        if (f_accumulator_array != nullptr)
        {
            delete f_accumulator_array;
            f_accumulator_array = nullptr;
        }
    }

    // node interface methods
    void power_averager::initialize()
    {
        out_buffer< 0 >().initialize( f_num_output_buffers )
        if (f_accumulator_array != nullptr)
        {
            LWARN( flog, "accululator array already exists" );
        }
        else
        {
            f_accumulator_array = new double[f_spectrum_size];
        }
    }

    void power_averager::execute( midge::diptera* a_midge )
    {
        try
        {
            //unsigned t_received = 0;
            while (! is_canceled() )
            {
                // Check for midge instructions
                if( have_instruction() )
                {
                    //LDEBUG( flog, "WARNING: power_averager does not support any instructions");
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
                    default: throw psyllid::error() << "input index <" << f_input_index << "> not recognized";
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

    void power_averager::finalize()
    {
    }

    /* power_averager_binding class */
    /***********************************/
    // power_averager_binding methods
    power_averager_binding::power_averager_binding()
    {
    }

    power_averager_binding::~power_averager_binding()
    {
    }

    void power_averager_binding::do_apply_config(power_averager* a_node, const scarab::param_node& a_config ) const
    {
        a_node->set_num_output_buffers( a_config.get_value( "num-output-buffers", a_node->get_num_output_buffers() ) );
        a_node->set_spectrum_size( a_config.get_value( "spectrum-size", a_node->get_spectrum_size() ) );
    }

    void power_averager_binding::do_dump_config( const power_averager* a_node, scarab::param_node& a_config ) const
    {
        a_config.add( "num-output-buffers", scarab::param_value( a_node->get_num_output_buffers() ) );
        a_config.add( "spectrum-size", scarab::param_value( a_node->get_spectrum_size() ) );
    }

} /* namespace fast_daq */

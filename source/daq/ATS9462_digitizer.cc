/*
 * ATS9464.cc
 *
 * Created on: Nov. 28, 2018
 *     Author: laroque
 */

#include <stdio.h>

#include "time_data.hh"

#include "ATS9462_digitizer.hh"


using midge::stream;

namespace fast_daq
{
    REGISTER_NODE_AND_BUILDER( ats9462_digitizer, "ats9462", ats9462_digitizer_binding );

    // ats9462_digitizer methods
    ats9462_digitizer::ats9462_digitizer() :
        f_samples_per_sec( 180000000.0 ),
        f_acquisition_length_sec( 0.1 ),
        f_samples_per_buffer( 204800 ),
        f_dma_buffer_count( 4883 ),
        f_system_id( 1 ),
        f_board_id( 1 ),
        f_channel_count( 1 ),
        f_bits_per_sample(),
        f_max_samples_per_channel(),
        f_out_length(),
        f_trigger_delay_sec(),
        f_trigger_timeout_sec(0.01)
    {
        f_board_handle = AlazarGetBoardBySystemID( f_system_id, f_board_id );
        if (f_board_handle == NULL)
        {
            printf("Error: Unable to open board system Id %u board Id %u\n", f_system_id, f_board_id);
            //TODO do something smarter here
            throw 1;
        }
        RETURN_CODE ret_code = AlazarGetChannelInfo(f_board_handle, &f_max_samples_per_channel, &f_bits_per_sample);
        if (ret_code != ApiSuccess)
        {
            printf("Error: AlazarGetChannelInfo failed -- %s\n", AlazarErrorToText(ret_code));
            //TODO do something smarter here
            throw 1;
        }
    }

    ats9462_digitizer::~ats9462_digitizer()
    {
        //TODO: critical that we make sure that the DMA buffers are cleaned up.
    }

    // node interface methods
    void ats9462_digitizer::initialize()
    {
        // setup output buffer
        out_buffer< 0 >().initialize( f_out_length );
        // configure the digitizer board
        configure_board();
    }

    void ats9462_digitizer::execute( midge::diptera* a_midge )
    {
        try
        {
            //TODO something interesting here?
        }
        catch( std::exception )
        {
            a_midge->throw_ex( std::current_exception() );
        }
    }

    void ats9462_digitizer::finalize()
    {
        out_buffer< 0 >().finalize();
    }

    bool ats9462_digitizer::check_return_code(RETURN_CODE a_return_code, std::string an_action)
    {
        if (a_return_code != ApiSuccess)
        {
            printf("Error: %s failed -- %s\n", an_action.c_str(), AlazarErrorToText(a_return_code));
            return false;
        }
        return true;
    }

    void ats9462_digitizer::configure_board()
    {
        RETURN_CODE ret_code;
        //TODO: again, here the sample rate is hard-coded, should be configured and f_samples_per_sec tied to the enum value selected
        ret_code = AlazarSetCaptureClock( f_board_handle, INTERNAL_CLOCK, SAMPLE_RATE_180MSPS, CLOCK_EDGE_RISING, 0);
        if ( ! check_return_code( ret_code, "AlazarSetCaptureClock" ) )
        {
            throw 1; //TODO ????
        }

        ret_code = AlazarInputControlEx( f_board_handle, CHANNEL_A, DC_COUPLING, INPUT_RANGE_PM_800_MV, IMPEDANCE_50_OHM );
        if ( ! check_return_code( ret_code, "AlazarInputControlEx" ) )
        {
            throw 1; //TODO???
        }

        ret_code = AlazarSetBWLimit( f_board_handle, CHANNEL_A, 0 );
        if ( ! check_return_code( ret_code, "AlazarSetBWLimit" ) )
        {
            throw 1; //TODO???
        }

        ret_code = AlazarSetTriggerOperation( f_board_handle,
                                              TRIG_ENGINE_OP_J,
                                              TRIG_ENGINE_J,
                                              TRIG_CHAN_A,
                                              TRIGGER_SLOPE_POSITIVE,
                                              150,
                                              TRIG_ENGINE_K,
                                              TRIG_DISABLE,
                                              TRIGGER_SLOPE_POSITIVE,
                                              128);
        if ( ! check_return_code( ret_code, "AlazarSetTriggerOperation" ) )
        {
            throw 1; //TODO???
        }

        ret_code = AlazarSetExternalTrigger( f_board_handle, DC_COUPLING, ETR_5V );
        if ( ! check_return_code( ret_code, "AlazarSetExternalTrigger" ) )
        {
            throw 1; //TODO???
        }

        U32 trigger_delay_samples = (U32)(f_trigger_delay_sec * f_samples_per_sec + 0.5);
        ret_code = AlazarSetTriggerDelay( f_board_handle, trigger_delay_samples );
        if ( ! check_return_code( ret_code, "AlazarSetTriggerDelay" ) )
        {
            throw 1; //TODO???
        }

        U32 trigger_timeout_clocks = (U32)(f_trigger_timeout_sec / 10.e-6 + 0.5);
        ret_code = AlazarSetTriggerTimeOut( f_board_handle, trigger_timeout_clocks );
        if ( ! check_return_code( ret_code, "AlazarSetTriggerTimeOut" ) )
        {
            throw 1; //TODO???
        }

        ret_code = AlazarConfigureAuxIO( f_board_handle, AUX_OUT_TRIGGER, 0 );
        if ( ! check_return_code( ret_code, "AlazarConfigureAuxIO" ) )
        {
            throw 1; //TODO???
        }
    }

    // Derived properties
    INT64 ats9462_digitizer::samples_per_acquisition()
    {
        return (INT64)(f_samples_per_sec * f_acquisition_length_sec + 0.5);
    }

    float ats9462_digitizer::bytes_per_sample()
    {
        return (float)((f_bits_per_sample + 7) / 8);
    }

    U32 ats9462_digitizer::bytes_per_buffer()
    {
        return (U32)(bytes_per_sample() * f_samples_per_buffer * f_channel_count + 0.5);
    }

    U32  ats9462_digitizer::buffers_per_acquisition()
    {
        return (U32)((samples_per_acquisition() + f_samples_per_buffer -1) / f_samples_per_buffer);
    }

    // ats9462_digitizer_binding methods
    ats9462_digitizer_binding::ats9462_digitizer_binding()
    {
    }

    ats9462_digitizer_binding::~ats9462_digitizer_binding()
    {
    }

    void ats9462_digitizer_binding::do_apply_config(ats9462_digitizer* a_node, const scarab::param_node& a_config ) const
    {
        a_node->set_samples_per_buffer( a_config.get_value( "samples-per-buffer", a_node->get_samples_per_buffer() ) );
        a_node->set_out_length( a_config.get_value( "out-length", a_node->get_out_length() ) );
    }

    void ats9462_digitizer_binding::do_dump_config( const ats9462_digitizer* a_node, scarab::param_node& a_config ) const
    {
        a_config.add( "samples-per-bufer", scarab::param_value( a_node->get_samples_per_buffer() ) );
        a_config.add( "out-length", scarab::param_value( a_node->get_out_length() ) );
    }

} /* namespace fast_daq */

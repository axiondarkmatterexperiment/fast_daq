/*
 * ATS9464.cc
 *
 * Created on: Nov. 28, 2018
 *     Author: laroque
 */

#include <stdio.h>

#include "daq_control.hh"
#include "real_time_data.hh"

#include "ATS9462_digitizer.hh"


using midge::stream;

namespace fast_daq
{
    REGISTER_NODE_AND_BUILDER( ats9462_digitizer, "ats9462", ats9462_digitizer_binding );

    LOGGER( flog, "ats9462_digitizer" );

    /* ats9462_digitizer class */
    /***************************/

    // ats9462_digitizer methods
    ats9462_digitizer::ats9462_digitizer() :
        f_samples_per_sec( 50000000 ), //default is 50MS/s
        f_acquisition_length_sec( 0.1 ),
        f_samples_per_buffer( 204800 ),
        f_input_mag_range( 400 ), // in +/- mV, must be a valid value from the enum
        f_dma_buffer_count( 4883 ),
        f_system_id( 1 ),
        f_board_id( 1 ),
        f_out_length(),
        f_trigger_delay_sec(),
        f_trigger_timeout_sec( 0.01 ),
        f_sample_rate_to_code(),
        f_channel_count( 1 ),
        f_channel_mask( CHANNEL_A ),
        f_bits_per_sample(),
        f_max_samples_per_channel(),
        f_paused( true ),
        f_board_buffers(),
        f_buffers_completed( 0 )
    {
        set_internal_maps();
        f_board_handle = AlazarGetBoardBySystemID( f_system_id, f_board_id );
        if (f_board_handle == NULL)
        {
            LERROR( flog, "Error: Unable to open board system Id " << f_system_id << " board Id " << f_board_id );
            //TODO do something smarter here
            throw 1;
        }
        RETURN_CODE ret_code = AlazarGetChannelInfo(f_board_handle, &f_max_samples_per_channel, &f_bits_per_sample);
        if (ret_code != ApiSuccess)
        {
            LERROR( flog, "Error: AlazarGetChannelInfo failed -- " << AlazarErrorToText(ret_code));
            //TODO do something smarter here
            throw 1;
        }
    }

    void ats9462_digitizer::set_internal_maps()
    {
        // see /usr/include/AlazarCmd.h enums for a full list of valid values

        // Sampling rates
        f_sample_rate_to_code.insert( rate_mapping_t( 1000, SAMPLE_RATE_1KSPS ) );
        f_sample_rate_to_code.insert( rate_mapping_t( 2000, SAMPLE_RATE_2KSPS ) );
        f_sample_rate_to_code.insert( rate_mapping_t( 5000, SAMPLE_RATE_5KSPS ) );
        f_sample_rate_to_code.insert( rate_mapping_t( 10000, SAMPLE_RATE_10KSPS ) );
        f_sample_rate_to_code.insert( rate_mapping_t( 20000, SAMPLE_RATE_20KSPS ) );
        f_sample_rate_to_code.insert( rate_mapping_t( 50000, SAMPLE_RATE_50KSPS ) );
        f_sample_rate_to_code.insert( rate_mapping_t( 100000, SAMPLE_RATE_100KSPS ) );
        f_sample_rate_to_code.insert( rate_mapping_t( 200000, SAMPLE_RATE_200KSPS ) );
        f_sample_rate_to_code.insert( rate_mapping_t( 500000, SAMPLE_RATE_500KSPS ) );
        f_sample_rate_to_code.insert( rate_mapping_t( 1000000, SAMPLE_RATE_1MSPS ) );
        f_sample_rate_to_code.insert( rate_mapping_t( 2000000, SAMPLE_RATE_2MSPS ) );
        f_sample_rate_to_code.insert( rate_mapping_t( 5000000, SAMPLE_RATE_5MSPS ) );
        f_sample_rate_to_code.insert( rate_mapping_t( 10000000, SAMPLE_RATE_10MSPS ) );
        f_sample_rate_to_code.insert( rate_mapping_t( 20000000, SAMPLE_RATE_20MSPS ) );
        // 25MSPS is in the header, but doesn't seem to work with our board...
        //f_sample_rate_to_code.insert( rate_mapping_t( 25000000, SAMPLE_RATE_25MSPS ) );
        f_sample_rate_to_code.insert( rate_mapping_t( 50000000, SAMPLE_RATE_50MSPS ) );
        f_sample_rate_to_code.insert( rate_mapping_t( 100000000, SAMPLE_RATE_100MSPS ) );
        f_sample_rate_to_code.insert( rate_mapping_t( 125000000, SAMPLE_RATE_125MSPS ) );
        f_sample_rate_to_code.insert( rate_mapping_t( 160000000, SAMPLE_RATE_160MSPS ) );
        f_sample_rate_to_code.insert( rate_mapping_t( 180000000, SAMPLE_RATE_180MSPS ) );

        // input ranges (note that left values are mV)
        f_input_range_to_code.insert( input_range_mapping_t( 400, INPUT_RANGE_PM_400_MV ) );
        f_input_range_to_code.insert( input_range_mapping_t( 800, INPUT_RANGE_PM_800_MV ) );
    }

    ats9462_digitizer::~ats9462_digitizer()
    {
        clear_buffers();
    }

    // node interface methods
    void ats9462_digitizer::initialize()
    {
        // setup output buffer
        out_buffer< 0 >().initialize( f_out_length );
        out_buffer< 0 >().call( &real_time_data::allocate_array, f_samples_per_buffer );
        //Convert +/- mV to dynamic range: (*2 for +/- /1000 for mV->V)
        out_buffer< 0 >().call( &real_time_data::set_dynamic_range, 2. * static_cast<double>(f_input_mag_range) / 1000. );
        // configure the digitizer board
        configure_board();
        allocate_buffers();
    }

    void ats9462_digitizer::execute( midge::diptera* a_midge )
    {
        try
        {
            //TODO something interesting here?
            while (! is_canceled() )
            {
                // Check for stop signal
                if( (out_stream< 0 >().get() == midge::stream::s_stop) )
                {
                    LINFO( flog, "Output stream has stop condition; break execution loop");
                    break;
                }
                // Check for midge instructions
                if( have_instruction() )
                {
                    process_instructions();
                }
                // If not paused continue to process
                if ( ! f_paused )
                {
                    //TODO some condition that if the run is complete
                    if ( f_buffers_completed >= buffers_per_acquisition() )
                    {
                        LINFO( flog, "All requested buffers ("<<f_buffers_completed<<") completed, calling stop_run" );
                        std::shared_ptr< psyllid::daq_control > t_daq_control = use_daq_control();
                        t_daq_control->stop_run();
                    }
                    else
                    {
                        process_a_buffer();
                    }
                }
            }
        }
        catch( std::exception )
        {
            a_midge->throw_ex( std::current_exception() );
        }
    }

    void ats9462_digitizer::finalize()
    {
        LDEBUG( flog, "in finalize... ");
        clear_buffers();
        out_buffer< 0 >().finalize();
    }

    bool ats9462_digitizer::check_return_code(RETURN_CODE a_return_code, std::string an_action, unsigned to_throw)
    {
        if (a_return_code != ApiSuccess)
        {
            LERROR( flog, "Error: " << an_action << " failed -- " << AlazarErrorToText(a_return_code));
            if (to_throw > 0)
            {
                throw to_throw; //TODO is this really the right thing to be doing?
            }
            return false;
        }
        return true;
    }

    void ats9462_digitizer::configure_board()
    {
        ALAZAR_SAMPLE_RATES this_rate = f_sample_rate_to_code.left.at( f_samples_per_sec );
        check_return_code( AlazarSetCaptureClock( f_board_handle, INTERNAL_CLOCK, this_rate, CLOCK_EDGE_RISING, 0),
                          "AlazarSetCaptureClock", 1 );

        ALAZAR_INPUT_RANGES this_input_range = f_input_range_to_code.left.at( f_input_mag_range );
        //check_return_code( AlazarInputControlEx( f_board_handle, CHANNEL_A, DC_COUPLING, INPUT_RANGE_PM_800_MV, IMPEDANCE_50_OHM ),
        //check_return_code( AlazarInputControlEx( f_board_handle, CHANNEL_A, DC_COUPLING, INPUT_RANGE_PM_400_MV, IMPEDANCE_50_OHM ),
        check_return_code( AlazarInputControlEx( f_board_handle, CHANNEL_A, DC_COUPLING, this_input_range, IMPEDANCE_50_OHM ),
                          "AlazarInputControlEx", 1 );

        check_return_code( AlazarSetBWLimit( f_board_handle, CHANNEL_A, 0 ),
                          "AlazarSetBWLimit", 1 );

        check_return_code( AlazarSetTriggerOperation( f_board_handle,
                                              TRIG_ENGINE_OP_J,
                                              TRIG_ENGINE_J,
                                              TRIG_CHAN_A,
                                              TRIGGER_SLOPE_POSITIVE,
                                              150,
                                              TRIG_ENGINE_K,
                                              TRIG_DISABLE,
                                              TRIGGER_SLOPE_POSITIVE,
                                              128),
                           "AlazarSetTriggerOperation", 1 );

        check_return_code( AlazarSetExternalTrigger( f_board_handle, DC_COUPLING, ETR_5V ),
                          "AlazarSetExternalTrigger", 1 );

        U32 trigger_delay_samples = (U32)(f_trigger_delay_sec * double(f_samples_per_sec) + 0.5);
        check_return_code( AlazarSetTriggerDelay( f_board_handle, trigger_delay_samples ),
                          "AlazarSetTriggerDelay", 1 );

        U32 trigger_timeout_clocks = (U32)(f_trigger_timeout_sec / 10.e-6 + 0.5);
        check_return_code( AlazarSetTriggerTimeOut( f_board_handle, trigger_timeout_clocks ),
                          "AlazarSetTriggerTimeOut", 1 );

        check_return_code( AlazarConfigureAuxIO( f_board_handle, AUX_OUT_TRIGGER, 0 ),
                          "AlazarConfigureAuxIO", 1 );
    }

    void ats9462_digitizer::allocate_buffers()
    {
        LINFO( flog, "allocating DMA buffers" );
        for (uint buffer_index = 0; buffer_index<f_dma_buffer_count; buffer_index++)
        {
            //TODO need to make sure that bytes_per_buffer cannot be changed via anything configurable, unless this is redone
            f_board_buffers.push_back( (U16*)valloc(bytes_per_buffer()) );
            if (f_board_buffers.back() == NULL)
            {
                LERROR( flog, "Error: unable to allocate buffer" );
                clear_buffers();
                throw 1;
            }
        }
    }

    void ats9462_digitizer::clear_buffers()
    {
        LDEBUG( flog, "clearing up DMA buffers" );
        check_return_code( AlazarAbortAsyncRead( f_board_handle ),
                          "AlazarAbortAsyncRead", 1 );
        LDEBUG( flog, "async read abort sent to board" );
        for (std::vector<U16*>::iterator a_buffer=f_board_buffers.begin(); a_buffer != f_board_buffers.end(); )
        {
            if (*a_buffer != nullptr)
            {
                free(*a_buffer);
            }
            f_board_buffers.erase(a_buffer);
        }
    }

    void ats9462_digitizer::process_instructions()
    {
        if( f_paused && use_instruction() == midge::instruction::resume )
        {
            LINFO( flog, "ATS9462 digitizer resuming");
            if( ! out_stream< 0 >().set( midge::stream::s_start ) ) throw midge::node_nonfatal_error() << "Stream 0 error while starting";
            f_buffers_completed = 0;
            f_paused = false;
            LINFO( flog, "run status members set" );
            //TODO something here to "start" a run
            //initial run setup for the board
            U32 adma_flags = ADMA_EXTERNAL_STARTCAPTURE | ADMA_TRIGGERED_STREAMING;
            check_return_code( AlazarBeforeAsyncRead( f_board_handle, f_channel_mask,
                                                          0, //per example, "Must be 0"
                                                          f_samples_per_buffer,
                                                          1, //per example, "Must be 1"
                                                          0x7FFFFFFF, //per example "Ignored. Behave as if infinite"
                                                          adma_flags
                                                        ),
                              "AlazarBeforeAsyncRead", 1 );
            LINFO( flog, "board pre-read complete" );
            //give the board all buffers
            for (U16* a_buffer : f_board_buffers)
            {
                check_return_code( AlazarPostAsyncBuffer( f_board_handle, a_buffer, bytes_per_buffer() ),
                                  "AlazarPostAsyncBuffer", 1 );
            }
            LINFO( flog, "buffers posted to board" );
            //arm the trigger, should start immediately
            check_return_code( AlazarStartCapture( f_board_handle ),
                              "AlazarStartCapture", 1 );
            LDEBUG( flog, "digitizer trigger armed, buffer collection should begin" );
        }
        else if( ! f_paused && use_instruction() == midge::instruction::pause )
        {
            LINFO( flog, "ATS9462 digitizer pausing");
            if( ! out_stream< 0 >().set( midge::stream::s_stop ) ) throw midge::node_nonfatal_error() << "Stream 0 error while stopping";
            f_paused = true;
            //TODO something here to "end" a run
            check_return_code( AlazarAbortAsyncRead( f_board_handle ),
                              "AlazarAbortAsyncRead", 1 );
            LDEBUG( flog, "abort async read sent to board" );
        }
        else
        {
            LWARN( flog, "unable to process message" );
        }
    }

    void ats9462_digitizer::process_a_buffer()
    {
        LTRACE( flog, "in process_a_buffer" );
        //grab the next buffer, once it is filled by the digitizer
        U16* this_buffer = f_board_buffers.at( f_buffers_completed % f_board_buffers.size() );
        check_return_code( AlazarWaitAsyncBufferComplete( f_board_handle, this_buffer, 5000 ),
                          "AlazarWaitAsyncBufferComplete", 1 );
        //copy the int array into the output stream
        real_time_data* time_data_out = out_stream< 0 >().data();
        //TODO remove this debugging thing
        std::vector<double> the_volts = time_data_out->as_volts();
        double max_V = *std::max_element(the_volts.begin(), the_volts.end());
        LWARN( flog, "ats max voltage in buffer is: " << max_V );
        //end TODO debugging
        std::memcpy( time_data_out->get_time_series(), &this_buffer[0], bytes_per_buffer() );
        if( !out_stream< 0 >().set( stream::s_run ) )
        {
            LERROR( flog, "error pushing time series to output stream" );
        }
        //return buffer to board and increment buffer count
        check_return_code( AlazarPostAsyncBuffer( f_board_handle, this_buffer, bytes_per_buffer() ),
                          "AlazarPostAsyncBuffer", 1 );
        f_buffers_completed++;
    }

    // Derived properties
    INT64 ats9462_digitizer::samples_per_acquisition()
    {
        return (INT64)(double(f_samples_per_sec) * f_acquisition_length_sec + 0.5);
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

    /* ats9462_digitizer_binding class */
    /***********************************/
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
        a_node->set_dma_buffer_count( a_config.get_value( "dma-buffer-count", a_node->get_dma_buffer_count() ) );
        a_node->set_samples_per_sec( a_config.get_value( "samples-per-sec", a_node->get_samples_per_sec() ) );
        a_node->set_acquisition_length_sec( a_config.get_value( "acquisition-length-sec", a_node->get_acquisition_length_sec() ) );
    }

    void ats9462_digitizer_binding::do_dump_config( const ats9462_digitizer* a_node, scarab::param_node& a_config ) const
    {
        a_config.add( "samples-per-bufer", scarab::param_value( a_node->get_samples_per_buffer() ) );
        a_config.add( "out-length", scarab::param_value( a_node->get_out_length() ) );
        a_config.add( "dma-buffer-count", scarab::param_value( a_node->get_dma_buffer_count() ) );
        a_config.add( "samples-per-sec", scarab::param_value( a_node->get_samples_per_sec() ) );
        a_config.add( "acquisition-length-sec", scarab::param_value( a_node->get_acquisition_length_sec() ) );
    }

} /* namespace fast_daq */

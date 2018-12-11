
/*
 * frequency_transform.cc
 *
 *  Created on: March 28, 2018
 *      Author: laroque
 */

#include "frequency_transform.hh"


#include "logger.hh"
#include "param.hh"

#include <cmath>
#include <thread>
#include <memory>
#include <sys/types.h>


using midge::stream;

namespace fast_daq
{
    REGISTER_NODE_AND_BUILDER( frequency_transform, "frequency-transform", frequency_transform_binding );

    LOGGER( plog, "frequency_transform" );

    // supporting enum helpers
    std::string frequency_transform::input_type_to_string( frequency_transform::input_type_t an_input_type )
    {
        switch (an_input_type) {
            case frequency_transform::input_type_t::real: return "real";
            case frequency_transform::input_type_t::complex: return "complex";
            default: throw psyllid::error() << "input_type value <" << input_type_to_uint(an_input_type) << "> not recognized";
        }
    }
    frequency_transform::input_type_t frequency_transform::string_to_input_type( const std::string& an_input_type )
    {
        if( an_input_type == input_type_to_string( frequency_transform::input_type_t::real ) ) return input_type_t::real;
        if( an_input_type == input_type_to_string( frequency_transform::input_type_t::complex ) ) return input_type_t::complex;
        throw psyllid::error() << "string <" << an_input_type << "> not recognized as valid input_type type";
    }


    // frequency_transform class implementation
    frequency_transform::frequency_transform() :
            f_time_length( 10 ),
            f_freq_length( 10 ),
            f_input_type( input_type_t::complex ),
            f_fft_size( 4096 ),
            f_transform_flag( "ESTIMATE" ),
            f_use_wisdom( true ),
            f_wisdom_filename( "wisdom_complexfft.fftw3" ),
            f_enable_time_output( false ), //Start with this false
            f_transform_flag_map(),
            f_fftw_input_real(),
            f_fftw_input_complex(),
            f_fftw_output(),
            f_fftw_plan(),
            f_multithreaded_is_initialized( false )
    {
        setup_internal_maps();
    }

    frequency_transform::~frequency_transform()
    {
    }

    // TODO this should be the only mode, the time output doesn't make sense here
    void frequency_transform::switch_to_freq_only()
    {
        LDEBUG( plog, "switching to frequency output only mode" );
        f_enable_time_output = false;
    }

    void frequency_transform::switch_to_time_and_freq()
    {
        LDEBUG( plog, "switching to frequency and time output mode" );
        LWARN( plog, "why are you switching on time output, that's not okay, we don't do that... throwing" );
        throw 1; //TODO: seems like the wrong thing to throw, but I'm intending to deprecate this entire behavior.
        f_enable_time_output = true;
    }

    void frequency_transform::initialize()
    {
        out_buffer< 0 >().initialize( f_freq_length );


        if (f_use_wisdom)
        {
            LDEBUG( plog, "Reading wisdom from file <" << f_wisdom_filename << ">");
            if (fftw_import_wisdom_from_filename(f_wisdom_filename.c_str()) == 0)
            {
                LWARN( plog, "Unable to read FFTW wisdom from file <" << f_wisdom_filename << ">" );
            }
        }
        //initialize multithreaded
        #ifdef FFTW_NTHREADS
            if (! f_multithreaded_is_initialized)
            {
                fftw_init_threads();
                fftw_plan_with_nthreads(FFTW_NTHREADS);
                LDEBUG( plog, "Configuring FFTW to use up to " << FFTW_NTHREADS << " threads.");
                f_multithreaded_is_initialized = true;
            }
        #endif
        // fftw stuff
        TransformFlagMap::const_iterator iter = f_transform_flag_map.find(f_transform_flag);
        unsigned transform_flag = iter->second;
        // initialize FFTW IO arrays and plan
        f_fftw_output = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * f_fft_size);
        switch (f_input_type)
        {
            case input_type_t::real:
                f_fftw_input_real = (double*) fftw_malloc(sizeof(double) * f_fft_size);
                f_fftw_plan = fftw_plan_dft_r2c_1d(f_fft_size, f_fftw_input_real, f_fftw_output, transform_flag | FFTW_PRESERVE_INPUT);
                break;
            case input_type_t::complex:
                f_fftw_input_complex = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * f_fft_size);
                f_fftw_plan = fftw_plan_dft_1d(f_fft_size, f_fftw_input_complex, f_fftw_output, FFTW_FORWARD, transform_flag | FFTW_PRESERVE_INPUT);
                break;
            default: throw psyllid::error() << "input_type not fully implemented";
        }
        //save plan
        if (f_fftw_plan != NULL)
        {
            if (f_use_wisdom)
            {
                if (fftw_export_wisdom_to_filename(f_wisdom_filename.c_str()) == 0)
                {
                    LWARN( plog, "Unable to write FFTW wisdom to file<" << f_wisdom_filename << ">");
                }
            }
            LDEBUG( plog, "FFTW plan created; initialization complete" );
        }

        return;
    }

    void frequency_transform::execute( midge::diptera* a_midge )
    {
        try
        {
            LDEBUG( plog, "Executing the frequency transformer" );

            psyllid::time_data* complex_time_data_in = nullptr;
            real_time_data* real_time_data_in = nullptr;
            //psyllid::time_data* time_data_out = nullptr;
            //psyllid::freq_data* freq_data_out = nullptr;
            frequency_data* freq_data_out = nullptr;
            double fft_norm = sqrt(1. / (double)f_fft_size);

            try
            {
                LINFO( plog, "Starting main loop (frequency transform)" );
                while (! is_canceled() )
                {
                    LDEBUG( plog, "check output stream signals" );
                    // stop if output stream buffers have s_stop
                    if (out_stream< 0 >().get() == stream::s_stop)
                    {
                        LWARN( plog, "frequency output stream has stop condition" );
                        break;
                    }
                    // grab the next input data and check slot status
                    LDEBUG( plog, "check input stream signals for <" << get_input_type_str() << ">" );
                    midge::enum_t in_cmd = stream::s_none;
                    unsigned in_stream_index = 0;
                    //unsigned in_stream_id = 0;
                    switch ( f_input_type )
                    {
                        case input_type_t::complex:
                            LDEBUG( plog, "seriously, getting 0" );
                            in_cmd = in_stream< 0 >().get();
                            in_stream_index = in_stream< 0 >().get_current_index();
                            //in_stream_id = 0;
                            break;
                        case input_type_t::real:
                            LDEBUG( plog, "seriously, getting 1" );
                            in_cmd = in_stream< 1 >().get();
                            in_stream_index = in_stream< 1 >().get_current_index();
                            //in_stream_id = 1;
                            break;
                    }
                    LDEBUG( plog, "input command is [" << in_cmd << "]");

                    if ( in_cmd == stream::s_none)
                    {
                        LDEBUG( plog, "got an s_none on slot <" << in_stream_index << ">" );
                        continue;
                    }
                    if ( in_cmd == stream::s_error )
                    {
                        LDEBUG( plog, "got an s_error on slot <" << in_stream_index << ">" );
                        break;
                    }
                    if ( in_cmd == stream::s_exit )
                    {
                        LDEBUG( plog, "got an s_exit on slot <" << in_stream_index << ">" );
                        break;
                    }
                    if ( in_cmd == stream::s_stop )
                    {
                        LDEBUG( plog, "got an s_stop on slot <" << in_stream_index << ">" );
                        if ( ! out_stream< 0 >().set( stream::s_stop ) ) throw midge::node_nonfatal_error() << "Stream 0 error while stopping";
                        continue;
                    }
                    if ( in_cmd == stream::s_start )
                    {
                        LDEBUG( plog, "got an s_start on slot <" << in_stream_index << ">" );
                        if ( ! out_stream< 0 >().set( stream::s_start ) ) throw midge::node_nonfatal_error() << "Stream 0 error while starting";
                        continue;
                    }
                    if ( in_cmd == stream::s_run )
                    {
                        LDEBUG( plog, "got an s_run on slot <" << in_stream_index << ">" );
                        unsigned t_center_bin;
                        switch ( f_input_type )
                        {
                            case input_type_t::real:
                                real_time_data_in = in_stream< 1 >().data();
                                t_center_bin = real_time_data_in->get_array_size();
                                break;
                            case input_type_t::complex:
                                complex_time_data_in = in_stream< 0 >().data();
                                t_center_bin = complex_time_data_in->get_array_size();
                                break;
                        }
                        LDEBUG( plog, "got input data" );

                        //frequency output
                        freq_data_out = out_stream< 0 >().data();
                        LDEBUG( plog, "next output stream slot acquired" );

                        switch (f_input_type)
                        {
                            case input_type_t::real:
                                LDEBUG( plog, "copy real input data" );
                                std::copy(&real_time_data_in->get_time_series()[0], &real_time_data_in->get_time_series()[0] + f_fft_size, &f_fftw_input_real[0]);
                                fftw_execute_dft_r2c(f_fftw_plan, f_fftw_input_real, f_fftw_output);
                                break;
                            case input_type_t::complex:
                                LDEBUG( plog, "grab complex data" );
                                std::copy(&complex_time_data_in->get_array()[0][0], &complex_time_data_in->get_array()[0][0] + f_fft_size*2, &f_fftw_input_complex[0][0]);
                                fftw_execute_dft(f_fftw_plan, f_fftw_input_complex, f_fftw_output);
                                break;
                            default: throw psyllid::error() << "input_type not fully implemented";
                        }
                        LDEBUG( plog, "executed the FFTW plan" );
                        //fftw_execute( f_fftw_plan );
                        //fftw_execute_dft(f_fftw_plan, f_fftw_input, f_fftw_output);

                        //take care of FFT normalization
                        //is this the normalization we want?
                        for (size_t i_bin=0; i_bin<f_fft_size; ++i_bin)
                        {
                            f_fftw_output[i_bin][0] *= fft_norm;
                            f_fftw_output[i_bin][1] *= fft_norm;
                        }

                        // FFT unfolding based on katydid:Source/Data/Transform/KTFrequencyTransformFFTW
//TODO remove these lines
LDEBUG( plog, "here's the fftw plan" );
fftw_print_plan( f_fftw_plan );
printf("\n");
                        //TODO here here the plan is broken by the next line... I think
                        //std::copy(&f_fftw_output[0][0], &f_fftw_output[0][0] + (t_center_bin - 1), &freq_data_out->get_data_array()[0][0] + t_center_bin);
                        // can I copy just 1 value to the front of the output array?
                        std::copy(&f_fftw_output[0][0], &f_fftw_output[0][1], &freq_data_out->get_data_array()[0][0] );
//TODO remove these lines
LDEBUG( plog, "here's the fftw plan" );
fftw_print_plan( f_fftw_plan );
printf("\n....\n");
                        //std::copy(&f_fftw_output[0][0] + t_center_bin, &f_fftw_output[0][0] + f_fft_size*2, &freq_data_out->get_data_array()[0][0]);
//TODO remove these lines
//LDEBUG( plog, "here's the fftw plan" );
//fftw_print_plan( f_fftw_plan );
//printf("\n");

                        if ( !out_stream< 0 >().set( stream::s_run ) )
                        {
                            LERROR( plog, "frequency_transform error setting frequency output stream to s_run" );
                            break;
                        }
                    }
                }
            }
            catch( psyllid::error& e )
            {
                throw;
            }

            LINFO( plog, "FREQUENCY TRANSFORM is exiting" );

            // normal exit condition
            LDEBUG( plog, "Stopping output stream" );
            bool t_f_stop_ok = out_stream< 0 >().set( stream::s_stop );
            if( ! t_f_stop_ok ) return;

            LDEBUG( plog, "Exiting output streams" );
            out_stream< 0 >().set( stream::s_exit );

            return;
        }
        catch(...)
        {
            if( a_midge ) a_midge->throw_ex( std::current_exception() );
            else throw;
        }

        return;
    }

    void frequency_transform::finalize()
    {
        LINFO( plog, "in finalize(), freeing fftw data objects" );
        out_buffer< 0 >().finalize();
        //out_buffer< 1 >().finalize();
        if (f_fftw_input_real != NULL )
        {
            fftw_free(f_fftw_input_real);
            f_fftw_input_real = NULL;
        }
        if (f_fftw_input_complex != NULL )
        {
            fftw_free(f_fftw_input_complex);
            f_fftw_input_complex = NULL;
        }
        if (f_fftw_output != NULL)
        {
            fftw_free(f_fftw_output);
            f_fftw_output = NULL;
        }
        return;
    }

    void frequency_transform::setup_internal_maps()
    {
        f_transform_flag_map.clear();
        f_transform_flag_map["ESTIMATE"] = FFTW_ESTIMATE;
        f_transform_flag_map["MEASURE"] = FFTW_MEASURE;
        f_transform_flag_map["PATIENT"] = FFTW_PATIENT;
        f_transform_flag_map["EXHAUSTIVE"] = FFTW_EXHAUSTIVE;
    }


    // frequency_transform_binding methods
    frequency_transform_binding::frequency_transform_binding() :
            _node_binding< frequency_transform, frequency_transform_binding >()
    {
    }

    frequency_transform_binding::~frequency_transform_binding()
    {
    }

    void frequency_transform_binding::do_apply_config( frequency_transform* a_node, const scarab::param_node& a_config ) const
    {
        LDEBUG( plog, "Configuring frequency_transform with:\n" << a_config );
        a_node->set_time_length( a_config.get_value( "time-length", a_node->get_time_length() ) );
        a_node->set_freq_length( a_config.get_value( "freq-length", a_node->get_freq_length() ) );
        a_node->set_input_type( a_config.get_value( "input-type", a_node->get_input_type_str() ) );
        a_node->set_fft_size( a_config.get_value( "fft-size", a_node->get_fft_size() ) );
        a_node->set_transform_flag( a_config.get_value( "transform-flag", a_node->get_transform_flag() ) );
        a_node->set_use_wisdom( a_config.get_value( "use-wisdom", a_node->get_use_wisdom() ) );
        a_node->set_wisdom_filename( a_config.get_value( "wisdom-filename", a_node->get_wisdom_filename() ) );
        return;
    }

    void frequency_transform_binding::do_dump_config( const frequency_transform* a_node, scarab::param_node& a_config ) const
    {
        LDEBUG( plog, "Dumping frequency_transform configuration" );
        a_config.add( "time-length", scarab::param_value( a_node->get_time_length() ) );
        a_config.add( "freq-length", scarab::param_value( a_node->get_freq_length() ) );
        a_config.add( "input-type", scarab::param_value( frequency_transform::input_type_to_string( a_node->get_input_type() ) ) );
        a_config.add( "fft-size", scarab::param_value( a_node->get_fft_size() ) );
        a_config.add( "transform-flag", scarab::param_value( a_node->get_transform_flag() ) );
        a_config.add( "use-wisdom", scarab::param_value( a_node->get_use_wisdom() ) );
        a_config.add( "wisdom-filename", scarab::param_value( a_node->get_wisdom_filename() ) );
        return;
    }

    bool frequency_transform_binding::do_run_command( frequency_transform* a_node, const std::string& a_cmd, const scarab::param_node& ) const
    {
        if ( a_cmd == "freq-only" )
        {
            LDEBUG( plog, "should enable freq-only mode" );
            a_node->switch_to_freq_only();
            return true;
        }
        else if ( a_cmd == "time-and-freq" )
        {
            LDEBUG( plog, "should enable time-and-freq mode" );
            a_node->switch_to_time_and_freq();
            return true;
        }
        else
        {
            LWARN( plog, "unrecognized command: <" << a_cmd << ">" );
            return false;
        }
    }

} /* namespace psyllid */

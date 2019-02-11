
/*
 * inverse_inverse_frequency_transform.cc
 *
 *  Created on: Feb. 6, 2019
 *      Author: laroque
 */

#include "inverse_frequency_transform.hh"

#include "logger.hh"
#include "param.hh"

#include <cmath>
#include <thread>
#include <memory>
#include <sys/types.h>


using midge::stream;

namespace fast_daq
{
    REGISTER_NODE_AND_BUILDER( inverse_frequency_transform, "inverse-frequency-transform", inverse_frequency_transform_binding );

    LOGGER( flog, "inverse_frequency_transform" );

    // inverse_frequency_transform class implementation
    inverse_frequency_transform::inverse_frequency_transform() :
            f_time_length( 10 ),
            f_fft_size( 4096 ),
            f_transform_flag( "ESTIMATE" ),
            f_use_wisdom( true ),
            f_wisdom_filename( "wisdom_complex_inversefft.fftw3" ),
            f_transform_flag_map(),
            f_fftw_input(),
            f_fftw_output(),
            f_fftw_plan(),
            f_multithreaded_is_initialized( false )
    {
        setup_internal_maps();
    }

    inverse_frequency_transform::~inverse_frequency_transform()
    {
    }

    void inverse_frequency_transform::initialize()
    {
        out_buffer< 0 >().initialize( f_time_length );
        out_buffer< 0 >().call( &iq_time_data::allocate_container, f_fft_size );

        if (f_use_wisdom)
        {
            LDEBUG( flog, "Reading wisdom from file <" << f_wisdom_filename << ">");
            if (fftw_import_wisdom_from_filename(f_wisdom_filename.c_str()) == 0)
            {
                LWARN( flog, "Unable to read FFTW wisdom from file <" << f_wisdom_filename << ">" );
            }
        }
        //initialize multithreaded
        #ifdef FFTW_NTHREADS
            if (! f_multithreaded_is_initialized)
            {
                fftw_init_threads();
                fftw_plan_with_nthreads(FFTW_NTHREADS);
                LDEBUG( flog, "Configuring FFTW to use up to " << FFTW_NTHREADS << " threads.");
                f_multithreaded_is_initialized = true;
            }
        #endif
        // fftw stuff
        transform_flag_map_t::const_iterator iter = f_transform_flag_map.find(f_transform_flag);
        unsigned transform_flag = iter->second;
        // initialize FFTW IO arrays and plan
        f_fftw_input= (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * f_fft_size);
        f_fftw_output = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * f_fft_size);
        f_fftw_plan = fftw_plan_dft_1d(f_fft_size, f_fftw_input, f_fftw_output, FFTW_BACKWARD, transform_flag | FFTW_PRESERVE_INPUT);
        //save plan
        if (f_fftw_plan != NULL)
        {
            if (f_use_wisdom)
            {
                if (fftw_export_wisdom_to_filename(f_wisdom_filename.c_str()) == 0)
                {
                    LWARN( flog, "Unable to write FFTW wisdom to file<" << f_wisdom_filename << ">");
                }
            }
            LDEBUG( flog, "FFTW plan created; initialization complete" );
        }
        return;
    }

    void inverse_frequency_transform::execute( midge::diptera* a_midge )
    {
        try
        {
            LDEBUG( flog, "Executing the frequency transformer" );

            frequency_data* input_freq_data = nullptr;
            iq_time_data* output_time_data = nullptr;

            try
            {
                LINFO( flog, "Starting main loop (frequency transform)" );
                while (! is_canceled() )
                {
                    // grab the next input data and check slot status
                    midge::enum_t in_cmd = stream::s_none;
                    unsigned in_stream_index = 0;
                    in_cmd = in_stream< 0 >().get();
                    in_stream_index = in_stream< 0 >().get_current_index();

                    if ( in_cmd == stream::s_none)
                    {
                        LDEBUG( flog, "got an s_none on slot <" << in_stream_index << ">" );
                        continue;
                    }
                    if ( in_cmd == stream::s_error )
                    {
                        LDEBUG( flog, "got an s_error on slot <" << in_stream_index << ">" );
                        break;
                    }
                    if ( in_cmd == stream::s_exit )
                    {
                        LDEBUG( flog, "got an s_exit on slot <" << in_stream_index << ">" );
                        break;
                    }
                    if ( in_cmd == stream::s_stop )
                    {
                        LDEBUG( flog, "got an s_stop on slot <" << in_stream_index << ">" );
                        //TODO extra timing report
                        LWARN( flog, "frequency output tim report" );
                        out_stream< 0 >().timer_report();
                        // end todo of extra timing prints
                        if ( ! out_stream< 0 >().set( stream::s_stop ) ) throw midge::node_nonfatal_error() << "Stream 0 error while stopping";
                        continue;
                    }
                    if ( in_cmd == stream::s_start )
                    {
                        LDEBUG( flog, "got an s_start on slot <" << in_stream_index << ">" );
                        if ( ! out_stream< 0 >().set( stream::s_start ) ) throw midge::node_nonfatal_error() << "Stream 0 error while starting";
                        continue;
                    }
                    if ( in_cmd == stream::s_run )
                    {
                        LTRACE( flog, "got an s_run on slot <" << in_stream_index << ">" );
                        // Grab data buffers for input and output streams
                        input_freq_data = in_stream< 0 >().data();
                        output_time_data = out_stream< 0 >().data();
                        output_time_data->set_chunk_counter( input_freq_data->get_chunk_counter() );
                        // copy input data into fft input array
                        std::copy(&input_freq_data->get_data_array()[0][0], &input_freq_data->get_data_array()[0][0] + 2*f_fft_size, &f_fftw_input[0][0] );

                        // execute fft
                        fftw_execute( f_fftw_plan );

                        //take care of FFT normalization
                        //is this the normalization we want?
                        double fft_norm = sqrt(1. / (double)f_fft_size);
                        for (size_t i_bin=0; i_bin<f_fft_size; ++i_bin)
                        {
                            f_fftw_output[i_bin][0] *= fft_norm;
                            f_fftw_output[i_bin][1] *= fft_norm;
                        }
                        // Is there anything weird in the output ordering of the inverse transform?
                        std::copy(&f_fftw_output[0][0], &f_fftw_output[f_fft_size][1], &output_time_data->get_data_array()[0][0]);
                        if ( !out_stream< 0 >().set( stream::s_run ) )
                        {
                            LERROR( flog, "inverse_frequency_transform error setting frequency output stream to s_run" );
                            break;
                        }
                    }
                }
            }
            catch( psyllid::error& e )
            {
                throw;
            }

            LINFO( flog, "FREQUENCY TRANSFORM is exiting" );

            // normal exit condition
            LDEBUG( flog, "Stopping output stream" );
            bool t_f_stop_ok = out_stream< 0 >().set( stream::s_stop );
            if( ! t_f_stop_ok ) return;

            LDEBUG( flog, "Exiting output streams" );
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

    void inverse_frequency_transform::finalize()
    {
        LINFO( flog, "in finalize(), freeing fftw data objects" );
        out_buffer< 0 >().finalize();
        if (f_fftw_input!= NULL )
        {
            fftw_free(f_fftw_input);
            f_fftw_input= NULL;
        }
        if (f_fftw_output != NULL)
        {
            fftw_free(f_fftw_output);
            f_fftw_output = NULL;
        }
        return;
    }

    void inverse_frequency_transform::setup_internal_maps()
    {
        f_transform_flag_map.clear();
        f_transform_flag_map["ESTIMATE"] = FFTW_ESTIMATE;
        f_transform_flag_map["MEASURE"] = FFTW_MEASURE;
        f_transform_flag_map["PATIENT"] = FFTW_PATIENT;
        f_transform_flag_map["EXHAUSTIVE"] = FFTW_EXHAUSTIVE;
    }


    // inverse_frequency_transform_binding methods
    inverse_frequency_transform_binding::inverse_frequency_transform_binding() :
            _node_binding< inverse_frequency_transform, inverse_frequency_transform_binding >()
    {
    }

    inverse_frequency_transform_binding::~inverse_frequency_transform_binding()
    {
    }

    void inverse_frequency_transform_binding::do_apply_config( inverse_frequency_transform* a_node, const scarab::param_node& a_config ) const
    {
        LDEBUG( flog, "Configuring inverse_frequency_transform with:\n" << a_config );
        a_node->set_time_length( a_config.get_value( "time-length", a_node->get_time_length() ) );
        a_node->set_fft_size( a_config.get_value( "fft-size", a_node->get_fft_size() ) );
        a_node->set_transform_flag( a_config.get_value( "transform-flag", a_node->get_transform_flag() ) );
        a_node->set_use_wisdom( a_config.get_value( "use-wisdom", a_node->get_use_wisdom() ) );
        a_node->set_wisdom_filename( a_config.get_value( "wisdom-filename", a_node->get_wisdom_filename() ) );
    }

    void inverse_frequency_transform_binding::do_dump_config( const inverse_frequency_transform* a_node, scarab::param_node& a_config ) const
    {
        LDEBUG( flog, "Dumping inverse_frequency_transform configuration" );
        a_config.add( "time-length", scarab::param_value( a_node->get_time_length() ) );
        a_config.add( "fft-size", scarab::param_value( a_node->get_fft_size() ) );
        a_config.add( "transform-flag", scarab::param_value( a_node->get_transform_flag() ) );
        a_config.add( "use-wisdom", scarab::param_value( a_node->get_use_wisdom() ) );
        a_config.add( "wisdom-filename", scarab::param_value( a_node->get_wisdom_filename() ) );
    }

    bool inverse_frequency_transform_binding::do_run_command( inverse_frequency_transform* /* a_node */, const std::string& a_cmd, const scarab::param_node& ) const
    {
        LWARN( flog, "unrecognized command: <" << a_cmd << ">" );
        return false;
    }

} /* namespace psyllid */

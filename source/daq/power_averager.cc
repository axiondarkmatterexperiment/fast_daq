/*
 * power_averager.cc
 *
 * Created on: Nov. 17, 2018
 *     Author: laroque
 */

#include <stdio.h>
#include <cmath>

//scarab includes
#include "logger.hh"

//fast_daq includes
#include "power_averager.hh"
#include "frequency_data.hh"
#include "power_data.hh"
#include "real_time_data.hh"


using midge::stream;

namespace fast_daq
{
    REGISTER_NODE_AND_BUILDER( power_averager, "power-averager", power_averager_binding );

    LOGGER( flog, "power_averager" );

    /* power_averager class */
    /***************************/

    // power_averager methods
    power_averager::power_averager() :
        f_num_output_buffers( 1 ),
        f_spectrum_size(),
        f_num_to_average( 0 ),
        f_bin_width(),
        f_minimum_frequency(),
        f_average_spectrum(),
        f_input_counter( 0 )
    {
    }

    power_averager::~power_averager()
    {
    }

    // node interface methods
    void power_averager::initialize()
    {
        out_buffer< 0 >().initialize( f_num_output_buffers );
        out_buffer< 0 >().call( &power_data::allocate_array, f_spectrum_size );
        f_average_spectrum.resize( f_spectrum_size, 0. );
    }

    void power_averager::execute( midge::diptera* a_midge )
    {
        try
        {
            while (! is_canceled() )
            {
                // Check for midge instructions
                if( have_instruction() )
                {
                    //LDEBUG( flog, "WARNING: power_averager does not support any instructions");
                }
                else { LWARN(flog, "no instruction" );}

                // check the slot status
                midge::enum_t input_command = in_stream< 0 >().get();
                unsigned stream_index = in_stream< 0 >().get_current_index();
                //stream_id = 0;
                if ( input_command == midge::stream::s_none )
                {
                    LDEBUG( flog, "who sends an s_none... what does that even mean?" );
                    continue;
                }
                else if ( input_command == stream::s_error )
                {
                    LWARN( flog, " got an s_error on slot <" << stream_index << ">, ... not doing anything about that." );
                    continue;
                }
                else if ( input_command == stream::s_stop )
                {
                    LINFO( flog, " got an s_stop on slot <" << stream_index << ">" );
                    handle_stop();
                    if ( ! out_stream< 0 >().set( midge::stream::s_stop ) ) throw midge::node_nonfatal_error() << "Stream 0 error while sending s_stop";
                    continue;
                }
                else if ( input_command == stream::s_start )
                {
                    LDEBUG( flog, " got an s_start on slot <" << stream_index << ">");
                    handle_start();
                    LDEBUG( flog, "start things done");
                    if ( ! out_stream< 0 >().set( midge::stream::s_start ) )
                    {
                        LERROR( flog, "unable to set start on output!" );
                        throw midge::node_nonfatal_error() << "Stream 0 error while sending s_start";
                    }
                    LWARN( flog, "start passed along" );
                    continue;
                }
                else if ( input_command == stream::s_run )
                {
                    LTRACE( flog, " got an s_run on slot <" << stream_index << ">");
                    handle_run();
                    continue;
                }
                LWARN( flog, "averager end of loop" );
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

    void power_averager::handle_start()
    {
        std::fill( f_average_spectrum.begin(), f_average_spectrum.end(), 0. );
        f_input_counter = 0;
    }

    void power_averager::handle_run()
    {
        frequency_data* data_in = in_stream< 0 >().data();
        frequency_data::complex_t* data_array_in = data_in->get_data_array();
        //TODO I shouldn't be doing this on each pass, just the first...
        //     ... even better, do it with a call upon getting s_start, or in init, or something
        f_bin_width = data_in->get_bin_width();
        f_minimum_frequency = data_in->get_minimum_frequency();
        if (data_in->get_array_size() != f_average_spectrum.size())
        {
            LERROR( flog, "input array size [" << data_in->get_array_size() <<"] != output array size ["<<f_average_spectrum.size()<<"]");
            //TODO throw something smart please
            throw 1;
        }
        for (unsigned i_bin=0; i_bin < data_in->get_array_size(); i_bin++)
        {
            // compute the power in mW (note, not W)
            // 1000.0 is to get to mW, 50.0 is impedance; 2.0 is to get RMS from peak voltage
            double these_mW = ( std::pow(data_array_in[i_bin][0], 2) + std::pow(data_array_in[i_bin][1], 2) ) * 1000.0 / 50.0 / 2.0;
            // divide by number of items in average and increment average spectrum buffer
            f_average_spectrum[i_bin] += these_mW / static_cast<double>(f_num_to_average);
        }
        f_input_counter++;
        if ( f_input_counter == f_num_to_average )
        {
            send_output();
        }
    }

    void power_averager::handle_stop()
    {
        LDEBUG( flog, " spectral data are:" );
        //TODO do I really want to send output data if the number of averaged values is not the expected number?
        if ( f_input_counter > 0 )
        {
            send_output();
        }
    }

    void power_averager::send_output()
    {
        // Rescale averaging N if needed
        if ( f_input_counter != f_num_to_average )
        {
            LWARN( flog, "number of collected points <" <<f_input_counter<< "> is not as expected (" <<f_num_to_average<< "), fixing average normalization" );
            // If number of collected points is less than expected average, rescale
            for (std::vector< double >::iterator bin_i = f_average_spectrum.begin(); bin_i != f_average_spectrum.end(); bin_i++)
            {
                *bin_i = (static_cast<double>(f_num_to_average) / static_cast<double>(f_input_counter)) * *bin_i;
            }
        }
        double power_max_mW = *std::max_element(f_average_spectrum.begin(), f_average_spectrum.end());
        LWARN( flog, "the maximum power bin has <" << power_max_mW << "> mW" );
        LWARN( flog, " ... <" << 10. * std::log10(power_max_mW) << "> dBm" );
        // Copy data into output stream and re-zero the averager container
        //power_data out_data = out_stream< 0 >().data();
        power_data* out_data_ptr = out_stream< 0 >().data();
        double* out_data_array = out_data_ptr->get_data_array();
        out_data_ptr->set_bin_width( f_bin_width );
        out_data_ptr->set_minimum_frequency( f_minimum_frequency );
        for (unsigned bin_i=0; bin_i < f_average_spectrum.size(); bin_i++)
        {
            out_data_array[bin_i] = f_average_spectrum[bin_i];
            f_average_spectrum[bin_i] = 0;
        }
        f_input_counter = 0;
        LINFO( flog, "sending out a spectrum" );
        if (!out_stream< 0 >().set( stream::s_run))
        {
            LERROR( flog, "unable to set s_run on output stream" );
            //TODO this should be something smarter
            throw 1;
        }
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
        a_node->set_num_to_average( a_config.get_value( "num-to-average", a_node->get_num_to_average() ) );
    }

    void power_averager_binding::do_dump_config( const power_averager* a_node, scarab::param_node& a_config ) const
    {
        a_config.add( "num-output-buffers", scarab::param_value( a_node->get_num_output_buffers() ) );
        a_config.add( "spectrum-size", scarab::param_value( a_node->get_spectrum_size() ) );
        a_config.add( "num-to-average", scarab::param_value( a_node->get_num_to_average() ) );
    }

} /* namespace fast_daq */

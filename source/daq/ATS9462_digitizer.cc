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
        f_max_samples_per_channel()
    {
        f_board = AlazarGetBoardBySystemID( f_system_id, f_board_id );
        if (f_board == NULL)
        {
            printf("Error: Unable to open board system Id %u board Id %u\n", f_system_id, f_board_id);
            //TODO do something smarter here
            throw 1;
        }
        RETURN_CODE ret_code = AlazarGetChannelInfo(f_board, &f_max_samples_per_channel, &f_bits_per_sample);
        if (ret_code != ApiSuccess)
        {
            printf("Error: AlazarGetChannelInfo failed -- %s\n", AlazarErrorToText(ret_code));
            throw 1;
        }
    }

    ats9462_digitizer::~ats9462_digitizer()
    {
    }

    // nodea interface methods
    void ats9462_digitizer::initialize()
    {
    }

    void ats9462_digitizer::execute( midge::diptera* a_midge )
    {
    }

    void ats9462_digitizer::finalize()
    {
    }

    void ats9462_digitizer::configure_board()
    {
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
    }

    void ats9462_digitizer_binding::do_dump_config( const ats9462_digitizer* a_node, scarab::param_node& a_config ) const
    {
    }

} /* namespace fast_daq */

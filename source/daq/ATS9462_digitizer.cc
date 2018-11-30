/*
 * ATS9464.cc
 *
 * Created on: Nov. 28, 2018
 *     Author: laroque
 */

#include <stdio.h>

#include "ATS9462_digitizer.hh"

namespace fast_daq
{
    ats9462_wrap::ats9462_wrap() :
        samples_per_sec( 180000000.0 ),
        acquisition_length_sec( 0.1 ),
        samples_per_buffer( 204800 ),
        dma_buffer_count( 4883 ),
        system_id( 1 ),
        board_id( 1 ),
        channel_count( 1 ),
        bits_per_sample(),
        max_samples_per_channel()
    {
        board = AlazarGetBoardBySystemID( system_id, board_id );
        if (board == NULL)
        {
            printf("Error: Unable to open board system Id %u board Id %u\n", system_id, board_id);
            //TODO do something smarter here
            throw 1;
        }
        RETURN_CODE ret_code = AlazarGetChannelInfo(board, &max_samples_per_channel, &bits_per_sample);
        if (ret_code != ApiSuccess)
        {
            printf("Error: AlazarGetChannelInfo failed -- %s\n", AlazarErrorToText(ret_code));
            throw 1;
        }
    }

    ats9462_wrap::~ats9462_wrap()
    {
    }

    void ats9462_wrap::configure_board()
    {
    }

    // Derived properties
    INT64 ats9462_wrap::samples_per_acquisition()
    {
        return (INT64)(samples_per_sec * acquisition_length_sec + 0.5);
    }

    float ats9462_wrap::bytes_per_sample()
    {
        return (float)((bits_per_sample + 7) / 8);
    }

    U32 ats9462_wrap::bytes_per_buffer()
    {
        return (U32)(bytes_per_sample() * samples_per_buffer * channel_count + 0.5);
    }

    U32  ats9462_wrap::buffers_per_acquisition()
    {
        return (U32)((samples_per_acquisition() + samples_per_buffer -1) / samples_per_buffer);
    }
} /* namespace fast_daq */

/*
 * ATS9464.hh
 *
 * Created on: Nov. 28, 2018
 *     Author: laroque
 */

#ifndef ATS9462_WRAP_HH_
#define ATS9462_WRAP_HH_

#include "AlazarError.h"
#include "AlazarApi.h"
#include "AlazarCmd.h"

namespace fast_daq
{
    class ats9462_wrap
    {
        public:
            ats9462_wrap();
            virtual ~ats9462_wrap();

        private:
            // These should use the scarab mv_* macros once I migrate
            double samples_per_sec;
            double acquisition_length_sec;
            U32 samples_per_buffer;
            U32 dma_buffer_count;
            U32 system_id;
            U32 board_id;
            U8 channel_count;
            U8 bits_per_sample;
            U32 max_samples_per_channel;
        private:
            HANDLE board;

        private:
            void configure_board();

        public:
            // Derived properties
            float bytes_per_sample();
            U32 bytes_per_buffer();
            INT64 samples_per_acquisition();
            U32 buffers_per_acquisition();

    };
} /* namespace fast_daq */
#endif /* ATS9462_WRAP_HH_ */

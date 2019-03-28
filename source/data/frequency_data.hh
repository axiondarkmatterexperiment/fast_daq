/*
 * frequency_data.hh
 *
 * Created on: Dec 11, 2018
 *     Author: laroque
 */

#ifndef FREQUENCY_DATA_HH_
#define FREQUENCY_DATA_HH_

#include "member_variables.hh"

namespace fast_daq
{
    class frequency_data
    {
        public:
            frequency_data();
            virtual ~frequency_data();

        public:
            typedef float complex_t[2];

        // member varaible macros
        mv_accessible( complex_t*, data_array );
        mv_accessible( unsigned, array_size ); // the length of the f_data_array
        mv_accessible( unsigned, fft_size ); // the length of the data array which went into the fft to produce this data (>= array_size)
        mv_accessible( float, bin_width ); // in [Hz]
        mv_accessible( float, minimum_frequency ); // in [Hz]
        mv_accessible( unsigned, chunk_counter );

        public:
            void allocate_array( unsigned n_samples );

    };
} /* namespace fast_daq */

#endif /* FREQUENCY_DATA_HH_ */

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
            typedef double complex_t[2];

        // member varaible macros
        mv_accessible( complex_t*, data_array );
        mv_accessible( unsigned, array_size);

        public:
            void allocate_array( unsigned n_samples );

    };
} /* namespace fast_daq */

#endif /* FREQUENCY_DATA_HH_ */

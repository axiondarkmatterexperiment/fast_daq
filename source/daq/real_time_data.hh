/*
 * real_time_data.hh
 *
 * Created on: Dec 4, 2018
 *     Author: laroque
 */

#ifndef REAL_TIME_DATA_HH_
#define REAL_TIME_DATA_HH_

#include "AlazarApi.h"
#include "member_variables.hh"

namespace fast_daq
{
    class real_time_data
    {
        public:
            real_time_data();
            virtual ~real_time_data();

        // member varaible macros
        mv_accessible( U16*, time_series );

        public:
            void allocate_array( unsigned n_samples );

    };
} /* namespace fast_daq */

#endif /* REAL_TIME_DATA_HH_ */

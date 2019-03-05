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

#include <vector>

namespace fast_daq
{
    /// This class contains "real" time data which is in some sense fundamenta
    class real_time_data
    {
        public:
            real_time_data();
            virtual ~real_time_data();

        // member varaible macros
        mv_accessible( U16*, time_series );
        mv_accessible( unsigned, array_size );
        mv_accessible( float, dynamic_range ); //full scale range in V (not mV; not magnitude)
        mv_accessible( std::vector<float>, volts_data );
        mv_accessible( unsigned, chunk_counter );

        public:
            void allocate_array( unsigned n_samples );
            // is this the right signature?
            std::vector<float> as_volts();

    };
} /* namespace fast_daq */

#endif /* REAL_TIME_DATA_HH_ */

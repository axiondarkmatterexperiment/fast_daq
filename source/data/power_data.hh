/*
 * power_data.hh
 *
 * Created on: Dec 11, 2018
 *     Author: laroque
 */

#ifndef POWER_DATA_HH_
#define POWER_DATA_HH_

#include "member_variables.hh"

namespace fast_daq
{
    class power_data
    {
        public:
            power_data();
            virtual ~power_data();

        // member varaible macros
        //TODO this should probably be an std::vector, not an float*
        mv_accessible( float*, data_array );
        mv_accessible( unsigned, array_size );
        mv_accessible( float, bin_width ); // in [Hz]
        mv_accessible( float, minimum_frequency ); // in [Hz]

        public:
            void allocate_array( unsigned n_samples );

    };
} /* namespace fast_daq */

#endif /* POWER_DATA_HH_ */

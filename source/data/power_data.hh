/*
 * power_data.hh
 *
 * Created on: Dec 11, 2018
 *     Author: laroque
 */

#ifndef POWER_DATA_HH_
#define POWER_DATA_HH_

#include "member_variables.hh"

#include <vector>

namespace fast_daq
{
    class power_data
    {
        public:
            power_data();
            virtual ~power_data() = default;

            power_data( const power_data& ) = delete;
            power_data& operator=( const power_data& ) = delete;

        mv_accessible( float, bin_width ); // in [Hz]
        mv_accessible( float, minimum_frequency ); // in [Hz]

        public:
            void allocate_array( unsigned n_samples );

            unsigned get_array_size() const { return f_data_array.size(); }

            float* get_data_array() { return f_data_array.data(); }
            const float* get_data_array() const { return f_data_array.data(); }

        private:
            std::vector< float > f_data_array;
    };
} /* namespace fast_daq */

#endif /* POWER_DATA_HH_ */

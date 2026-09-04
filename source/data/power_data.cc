/*
 * power_data.cc
 *
 * Created on: Dec 11, 2018
 *     Author: laroque
 */

#include "power_data.hh"

namespace fast_daq
{
    power_data::power_data() :
        f_bin_width(),
        f_minimum_frequency()
    {
    }

    void power_data::allocate_array( unsigned n_samples )
    {
        f_data_array.resize( n_samples, 0. );
    }
} /* namespace fast_daq */

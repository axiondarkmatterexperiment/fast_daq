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
        f_data_array(),
        f_array_size()
    {
    }

    power_data::~power_data()
    {
        if (f_data_array != nullptr)
        {
            delete f_data_array;
            f_data_array = nullptr;
        }
    }

    void power_data::allocate_array( unsigned n_samples )
    {
        if (f_data_array == nullptr )
        {
            f_data_array = new double[n_samples];
        }
        f_array_size = n_samples;
    }
} /* namespace fast_daq */

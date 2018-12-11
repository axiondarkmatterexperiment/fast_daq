/*
 * frequency_data.cc
 *
 * Created on: Dec 11, 2018
 *     Author: laroque
 */

#include "frequency_data.hh"

namespace fast_daq
{
    frequency_data::frequency_data() :
        f_data_array(),
        f_array_size()
    {
    }

    frequency_data::~frequency_data()
    {
        if (f_data_array != nullptr)
        {
            delete f_data_array;
            f_data_array = nullptr;
        }
    }

    void frequency_data::allocate_array( unsigned n_samples )
    {
        if (f_data_array == nullptr )
        {
            f_data_array = new complex_t[n_samples];
        }
        f_array_size = n_samples;
    }
} /* namespace fast_daq */

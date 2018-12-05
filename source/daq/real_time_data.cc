/*
 * real_time_data.cc
 *
 * Created on: Dec 4, 2018
 *     Author: laroque
 */

#include "real_time_data.hh"

namespace fast_daq
{
    real_time_data::real_time_data()
    {
    }

    real_time_data::~real_time_data()
    {
        if (f_time_series != nullptr)
        {
            delete f_time_series;
        }
    }

    void real_time_data::allocate_array( unsigned n_samples )
    {
        if (f_time_series == nullptr )
        {
            f_time_series = new U16[n_samples];
        }
    }
} /* namespace fast_daq */

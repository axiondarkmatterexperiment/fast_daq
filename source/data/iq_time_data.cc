/*
 * iq_time_data.cc
 *
 * Created on: Feb. 7, 2019
 *     Author: laroque
 */

#include "iq_time_data.hh"

namespace fast_daq
{
    iq_time_data::iq_time_data() :
        f_array_size(),
        f_data_array(),
        f_chunk_counter()
    {
    }

    iq_time_data::~iq_time_data()
    {
    }

    void iq_time_data::allocate_container( unsigned n_samples )
    {
        if ( f_data_array == nullptr )
        {
            f_data_array = new complex_t[n_samples];
        }
        f_array_size = n_samples;
    }

} /* namespace fast_daq */

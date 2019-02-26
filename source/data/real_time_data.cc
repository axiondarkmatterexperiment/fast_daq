/*
 * real_time_data.cc
 *
 * Created on: Dec 4, 2018
 *     Author: laroque
 */

#include "real_time_data.hh"

namespace fast_daq
{
    real_time_data::real_time_data() :
        f_time_series(),
        f_array_size(),
        f_dynamic_range(),
        f_volts_data(),
        f_chunk_counter()
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
        f_array_size = n_samples;
        f_volts_data.resize( n_samples );
    }

    std::vector<double> real_time_data::as_volts()
    {
         //std::vector<double> volts_data(f_array_size);
         double units_factor = f_dynamic_range / 65536.;
         double min_volts = f_dynamic_range / 2.0;
         for (unsigned i_bin=0; i_bin<f_array_size; ++i_bin)
         {
            //TODO validate this:
            // Note that volts = ((this_ADC_channel/number_of_channels) * dynamic_range) + min_voltage_in_range
            // ... where min_voltage_in_range == -(dynamic_range/2.)
            f_volts_data[i_bin] = (static_cast<double>(f_time_series[i_bin]) * units_factor) - min_volts;
         }
         return f_volts_data;
    }
} /* namespace fast_daq */

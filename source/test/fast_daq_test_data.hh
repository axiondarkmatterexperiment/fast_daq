#ifndef FAST_DAQ_TEST_DATA_HH_
#define FAST_DAQ_TEST_DATA_HH_

#include "frequency_data.hh"
#include "power_data.hh"
#include "real_time_data.hh"
#include "iq_time_data.hh"

#include <array>
#include <vector>

namespace fast_daq
{
    namespace testing
    {
        /// Deep copy of a power_data record -- safe to hold after the buffer is destroyed
        struct captured_power
        {
            std::vector< float > f_bins;
            float f_bin_width        = 0.f;
            float f_minimum_frequency = 0.f;
        };

        /// Deep copy of a frequency_data record
        struct captured_frequency
        {
            std::vector< std::array< float, 2 > > f_bins;
            float f_bin_width        = 0.f;
            float f_minimum_frequency = 0.f;
            unsigned f_fft_size      = 0;
            unsigned f_chunk_counter = 0;
        };

        /// Deep copy of a real_time_data record
        struct captured_real_time
        {
            std::vector< U16 > f_samples;
            float f_dynamic_range = 0.f;
            unsigned f_chunk_counter = 0;
        };

        // ---------------------------------------------------------------------------
        // In-place fill helpers -- these write into an already-allocated slot.
        // Never copy the payload type; always use the in-place pointer directly.
        // ---------------------------------------------------------------------------

        /// Fill a frequency_data slot with a constant complex value on every bin.
        inline void fill_frequency_constant( frequency_data* a_slot,
                                             unsigned a_size,
                                             float a_real,
                                             float a_imag,
                                             float a_bin_width = 100.f,
                                             float a_min_freq  = 0.f )
        {
            a_slot->allocate_array( a_size );
            a_slot->set_bin_width( a_bin_width );
            a_slot->set_minimum_frequency( a_min_freq );
            frequency_data::complex_t* arr = a_slot->get_data_array();
            for( unsigned i = 0; i < a_size; ++i )
            {
                arr[i][0] = a_real;
                arr[i][1] = a_imag;
            }
        }

        /// Fill a real_time_data slot with a constant U16 sample value.
        inline void fill_real_time_constant( real_time_data* a_slot,
                                             unsigned a_size,
                                             U16 a_value,
                                             float a_dynamic_range = 1.f )
        {
            a_slot->allocate_array( a_size );
            a_slot->set_dynamic_range( a_dynamic_range );
            for( unsigned i = 0; i < a_size; ++i )
                a_slot->get_time_series()[i] = a_value;
        }

        // ---------------------------------------------------------------------------
        // Deep-copy extractors -- safe to call from test_consumer::set_extract().
        // Each copies out of the live buffer slot into a caller-owned struct.
        // ---------------------------------------------------------------------------

        inline captured_power extract_power( const power_data* a_rec )
        {
            captured_power t_cap;
            const float* arr = a_rec->get_data_array();
            unsigned n = a_rec->get_array_size();
            t_cap.f_bins.assign( arr, arr + n );
            t_cap.f_bin_width         = a_rec->get_bin_width();
            t_cap.f_minimum_frequency = a_rec->get_minimum_frequency();
            return t_cap;
        }

        inline captured_frequency extract_frequency( const frequency_data* a_rec )
        {
            captured_frequency t_cap;
            unsigned n = a_rec->get_array_size();
            t_cap.f_bins.resize( n );
            const frequency_data::complex_t* arr = a_rec->get_data_array();
            for( unsigned i = 0; i < n; ++i )
            {
                t_cap.f_bins[i][0] = arr[i][0];
                t_cap.f_bins[i][1] = arr[i][1];
            }
            t_cap.f_bin_width         = a_rec->get_bin_width();
            t_cap.f_minimum_frequency = a_rec->get_minimum_frequency();
            t_cap.f_fft_size          = a_rec->get_fft_size();
            t_cap.f_chunk_counter     = a_rec->get_chunk_counter();
            return t_cap;
        }

        inline captured_real_time extract_real_time( const real_time_data* a_rec )
        {
            captured_real_time t_cap;
            unsigned n = a_rec->get_array_size();
            const U16* arr = a_rec->get_time_series();
            t_cap.f_samples.assign( arr, arr + n );
            t_cap.f_dynamic_range  = a_rec->get_dynamic_range();
            t_cap.f_chunk_counter  = a_rec->get_chunk_counter();
            return t_cap;
        }

    } // namespace testing
} // namespace fast_daq

#endif /* FAST_DAQ_TEST_DATA_HH_ */

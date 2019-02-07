/*
 * iq_time_data.hh
 *
 * Created on: Dec 4, 2018
 *     Author: laroque
 */

#ifndef IQ_TIME_DATA_HH_
#define IQ_TIME_DATA_HH_

//#include "AlazarApi.h"
#include "member_variables.hh"

#include <vector>
#include <complex>

namespace fast_daq
{
    /// This class contains "iq" time series which is naturally in units of Volts (not ADC units).
    class iq_time_data
    {
        public:
            iq_time_data();
            virtual ~iq_time_data();

        // member varaible macros
        mv_accessible( unsigned, array_size );
        //mv_accessible( std::vector< std::complex< double > >, volts_data );
        mv_accessible( std::complex< double >*, data_array );
        mv_accessible( unsigned, chunk_counter );

        public:
            void allocate_container( unsigned n_samples );

    };
} /* namespace fast_daq */

#endif /* IQ_TIME_DATA_HH_ */

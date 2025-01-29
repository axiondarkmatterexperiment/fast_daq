/*
 * inverse_frequency_transform.hh
 *
 *  Created on: Feb. 6. 2019
 *      Author: laroque
 */

#ifndef INVERSE_PSYLLID_FREQUENCY_TRANSFORM_HH_
#define INVERSE_PSYLLID_FREQUENCY_TRANSFORM_HH_

//psyllid
#include "node_builder.hh"
//#include "time_data.hh"

//fast_daq
#include "frequency_data.hh"
#include "iq_time_data.hh"

//midge
#include "transformer.hh"
#include "shared_cancel.hh"

//external
#include <fftw3.h>

namespace scarab
{
    class param_node;
}

namespace fast_daq
{
    /*!
     @class inverse_frequency_transform
     @author B. H. LaRoque

     @brief A transformer to receive frequency data, compute an inverse FFT, and distribute as time data

     @details

     Parameter setting is not thread-safe.  Executing is thread-safe.

     Node type: "inverse-freq-transform"

     Available configuration values:
     - "time-length": uint -- The size of the output time-data buffer
     - "fft-size": unsigned -- The length of the fft input/output array (each element is 2-component)
     - "transform-flag": string -- FFTW flag to indicate how much optimization of the fftwf_plan is desired
     - "use-wisdom": bool -- whether to use a plan from a wisdom file and save the plan to that file
     - "wisdom-filename": string -- if "use-wisdom" is true, resolvable path to the wisdom file

     Input Stream:
     - 0: frequency_data

     Output Streams:
     - 0: psyllid::time_data (IQ)
    */
    class inverse_frequency_transform : public midge::_transformer< midge::type_list< frequency_data >, midge::type_list< iq_time_data > >
    {
        public:
            // internal enums

        private:
            typedef std::map< std::string, unsigned > transform_flag_map_t;

        public:
            inverse_frequency_transform();
            virtual ~inverse_frequency_transform();

        // member variable macros
        mv_accessible( uint64_t, time_length );
        public:
            mv_accessible( unsigned, fft_size ); // I really wish I could get this the upstream node
            // the upstream node could put it on the data objects when it inits them
            mv_accessible( unsigned, fft_size_fraction );
            mv_accessible( std::string, transform_flag );
            mv_accessible( bool, use_wisdom );
            mv_accessible( std::string, wisdom_filename );
            mv_accessible( double, start_fraction );
            mv_accessible( double, stop_fraction );

        public:
            virtual void initialize();
            virtual void execute( midge::diptera* a_midge = nullptr );
            virtual void finalize();

        private:
            transform_flag_map_t f_transform_flag_map;
            fftwf_complex* f_fftwf_input;
            fftwf_complex* f_fftwf_output;
            fftwf_complex* f_fftwf_input_part;
            fftwf_plan f_fftwf_plan;

            bool f_multithreaded_is_initialized;

        private:
            void setup_internal_maps();

    };


    class inverse_frequency_transform_binding : public psyllid::_node_binding< inverse_frequency_transform, inverse_frequency_transform_binding >
    {
        public:
            inverse_frequency_transform_binding();
            virtual ~inverse_frequency_transform_binding();

        private:
            virtual void do_apply_config( inverse_frequency_transform* a_node, const scarab::param_node& a_config ) const;
            virtual void do_dump_config( const inverse_frequency_transform* a_node, scarab::param_node& a_config ) const;
            virtual bool do_run_command( inverse_frequency_transform* a_node, const std::string& a_cmd, const scarab::param_node& ) const;
    };

} /* namespace psyllid */

#endif /* PSYLLID_INVERSE_FREQUENCY_TRANSFORM_HH_ */

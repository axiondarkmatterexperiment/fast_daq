/*
 * frequency_transform.hh
 *
 *  Created on: March 28, 2018
 *      Author: laroque
 */

#ifndef PSYLLID_FREQUENCY_TRANSFORM_HH_
#define PSYLLID_FREQUENCY_TRANSFORM_HH_

//psyllid
//#include "freq_data.hh"
#include "node_builder.hh"
#include "time_data.hh"

//fast_daq
#include "frequency_data.hh"
#include "real_time_data.hh"

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
     @class frequency_transform
     @author B. H. LaRoque

     @brief A transformer to receive time data, compute an FFT, and distribute as time and frequency ROACH packets.

     @details

     Parameter setting is not thread-safe.  Executing is thread-safe.

     Node type: "frequency-transform"

     Available configuration values:
     - "time-length": uint -- The size of the output time-data buffer
     - "freq-length": uint -- The size of the output frequency-data buffer
     - "input-type": string -- must either by "real" or "complex" (corresponds to using input stream 1 or 0 respectively)
     - "fft-size": unsigned -- The length of the fft input/output array (each element is 2-component)
     - "transform-flag": string -- FFTW flag to indicate how much optimization of the fftw_plan is desired
     - "use-wisdom": bool -- whether to use a plan from a wisdom file and save the plan to that file
     - "wisdom-filename": string -- if "use-wisdom" is true, resolvable path to the wisdom file

    Available DAQ commands:
    - "freq-only" (no args) -- Switch the execution mode to frequency only
    - "time-and-freq" (no args) -- Switch the execution mode to time-and-frequency

     Input Stream:
     - 0: time_data (IQ)
     - 1: real_time_data

     Output Streams:
     - 0: frequency_data
    */
    class frequency_transform : public midge::_transformer< frequency_transform, typelist_2( psyllid::time_data, real_time_data ), typelist_1( frequency_data ) >
    {
        public:
            enum class input_type_t
            {
                real,
                complex
            };
            static uint32_t input_type_to_uint( input_type_t an_input_type );
            static input_type_t uint_to_input_type( uint32_t an_input_type_uint );
            static std::string input_type_to_string( input_type_t an_input_type );
            static input_type_t string_to_input_type( const std::string& an_input_type );

        private:
            typedef std::map< std::string, unsigned > TransformFlagMap;

        public:
            frequency_transform();
            virtual ~frequency_transform();
            void set_input_type( const std::string& an_input_type );

        // member variable macros
        mv_accessible( uint64_t, time_length );
        mv_accessible( uint64_t, freq_length );
        mv_accessible( input_type_t, input_type );
        public:
            std::string get_input_type_str() const;
        mv_accessible( unsigned, fft_size ); // I really wish I could get this from the egg header
        mv_accessible( std::string, transform_flag );
        mv_accessible( bool, use_wisdom );
        mv_accessible( std::string, wisdom_filename );

        private:
            bool f_enable_time_output;

        public:
            void switch_to_freq_only();
            void switch_to_time_and_freq();

            virtual void initialize();
            virtual void execute( midge::diptera* a_midge = nullptr );
            virtual void finalize();

        private:
            TransformFlagMap f_transform_flag_map;
            double* f_fftw_input_real;
            fftw_complex* f_fftw_input_complex;
            fftw_complex* f_fftw_output;
            fftw_plan f_fftw_plan;

            bool f_multithreaded_is_initialized;

            //uint64_t f_time_session_pkt_counter;
            //uint64_t f_freq_session_pkt_counter;
        private:
            void setup_internal_maps();

    };

    inline uint32_t frequency_transform::input_type_to_uint( input_type_t an_input_type )
    {
        return static_cast< uint32_t >( an_input_type );
    }
    inline frequency_transform::input_type_t frequency_transform::uint_to_input_type( uint32_t an_input_type_uint )
    {
        return static_cast< frequency_transform::input_type_t >( an_input_type_uint );
    }
    inline void frequency_transform::set_input_type( const std::string& an_input_type )
    {
        set_input_type( string_to_input_type( an_input_type ) );
    }
    inline std::string frequency_transform::get_input_type_str() const
    {
        return input_type_to_string( f_input_type );
    }
    /*
    inline std::string frequency_transform::input_type_to_string( input_type_t an_input_type )
    {
        return static_cast< uint32_t >( an_input_type );
    }
    inline frequency_transform::input_type_t frequency_transform::string_to_input_type( const std::string& an_input_type )
    {
        return static_cast< frequency_transform::input_type_t >( an_input_type );
    }
    inline void frequency_transform::set_input_type( const std::string& an_input_type )
    {
        set_input_type( string_to_input_type( an_input_type ) );
    }
    */

    class frequency_transform_binding : public psyllid::_node_binding< frequency_transform, frequency_transform_binding >
    {
        public:
            frequency_transform_binding();
            virtual ~frequency_transform_binding();

        private:
            virtual void do_apply_config( frequency_transform* a_node, const scarab::param_node& a_config ) const;
            virtual void do_dump_config( const frequency_transform* a_node, scarab::param_node& a_config ) const;
            virtual bool do_run_command( frequency_transform* a_node, const std::string& a_cmd, const scarab::param_node& ) const;
    };

} /* namespace psyllid */

#endif /* PSYLLID_FREQUENCY_TRANSFORM_HH_ */

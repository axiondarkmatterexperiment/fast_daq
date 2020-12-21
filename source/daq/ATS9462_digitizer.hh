/*
 * ATS9464_digitizer.hh
 *
 * Created on: Nov. 28, 2018
 *     Author: laroque
 */

#ifndef ATS9462_WRAP_HH_
#define ATS9462_WRAP_HH_

#include <boost/config.hpp>
#include <boost/bimap.hpp>

// AlazarTech includes
#include "AlazarError.h"
#include "AlazarApi.h"
#include "AlazarCmd.h"

// sandfly includes
#include "node_builder.hh"

#include "producer.hh"
#include "control_access.hh"
#include "fast_daq_error.hh"


#define check_return_code_macro( function, ... ) \
    ats9462_digitizer::check_return_code( function(__VA_ARGS__), STRINGIFY(function), __FILE_LINE__ );

namespace fast_daq
{
    // forward declarations
    class real_time_data;
    /*!
     @class ats9462_digitizer
     @author B. H. LaRoque

     @brief A producer to read time-domain data from an AlazarTech ATS9462 digitizer

     @details

     A node for continuous streaming of time samples from the digitizer. Many intuitive features are *not* currently supported:
     - there is no support for selecting inputs (only A is supported)
     - there is no support for driving the sampling using the external input
     - there is no support for making changes to the board's configuration after the initial startup (you can set values but they will not be applied and behavior is undefined).

     Node type: "ats9462"

     Available configuration values:
     - "samples-per-buffer": int -- number of real-valued samples to include in each chunk of data
     - "out-length": int -- number of output buffer slots
     - "dma-buffer-count": int -- the number of DMA buffers to use between the digitzer board and the application
     - "samples-per-sec": int -- number of samples per second (must be in the set of allowed rates in the digitizer library) (default=25000000)
     - "acquisition-length-sec": double -- the duration of the run in seconds (will be used to compute the integer number of buffers to collect)

     Output Streams
     - 0: real_time_data

    */
    class ats9462_digitizer : public midge::_producer< midge::type_list< real_time_data > >, public sandfly::control_access
    {
        public:
            enum class reference_source_t
            {
		internal = INTERNAL_CLOCK,
                external_10MHz = EXTERNAL_CLOCK_10MHZ_REF
            };
            static uint32_t reference_source_to_uint( reference_source_t a_reference_source );
            static reference_source_t uint_to_reference_source( uint32_t a_reference_source_uint );
            static std::string reference_source_to_string( reference_source_t a_reference_source );
            static reference_source_t string_to_reference_source( const std::string& a_reference_source );

        private:
            typedef boost::bimap< uint32_t, ALAZAR_SAMPLE_RATES > sample_rate_code_map_t;
            typedef sample_rate_code_map_t::value_type rate_mapping_t;
            // Note that the input range left values are [mV], ie input is +/- # mV
            typedef boost::bimap< uint32_t, ALAZAR_INPUT_RANGES > input_range_code_map_t;
            typedef input_range_code_map_t::value_type input_range_mapping_t;

        public:
            ats9462_digitizer();
            virtual ~ats9462_digitizer();
            void set_reference_source_and_decimation( const std::string& a_reference_source, U32 a_decimation_factor );

        private:
            void set_internal_maps();

        public: //node API
            virtual void initialize();
            virtual void execute( midge::diptera* a_midge = nullptr );
            virtual void finalize();

        //TODO implement custom setters that do not allow changes after the board has been configured (for those parameters set in board configuration)
        public:
            std::string get_reference_source_str() const;
        mv_accessible( U32, samples_per_sec );
	void set_reference_source_and_decimation( reference_source_t a_reference_source, U32 a_decimation_factor );
        mv_accessible_noset( reference_source_t, reference_source );
        mv_accessible_noset( U32, decimation_factor ); // note that this is the "decimation_value"+1 (ie, to decimate by 10 -> returned_samples_per_sec = physical_samples_per_sec / 10; the "decimation_factor" is 10, and the "decimation_value" is 9)
        mv_accessible( double, acquisition_length_sec );
        mv_accessible( U32, samples_per_buffer );
        mv_accessible( U32, input_mag_range );
        mv_accessible( U32, dma_buffer_count );
        mv_accessible( unsigned, next_read_buffer );
        mv_accessible( U32, system_id );
        mv_accessible( U32, board_id );
        mv_accessible( uint64_t, out_length );
        mv_accessible( double, trigger_delay_sec );
        mv_accessible( double, trigger_timeout_sec );
        mv_accessible( unsigned, chunk_counter );
        mv_accessible( unsigned, overrun_collected );

        private:
            sample_rate_code_map_t f_sample_rate_to_code;
            input_range_code_map_t f_input_range_to_code;
            U8 f_channel_count;
            U32 f_channel_mask;
            U8 f_bits_per_sample;
            U32 f_max_samples_per_channel;
            HANDLE f_board_handle;
            bool f_paused;
            std::vector<U16*> f_board_buffers;
            U32 f_buffers_completed;

        private:
            //bool check_return_code(RETURN_CODE a_return_code, std::string an_action, unsigned to_throw);
            static void check_return_code( RETURN_CODE a_return_code, const std::string& an_action, const std::string& a_file_line );//, const std::string& a_line );
            void configure_board();
            void allocate_buffers();
            void clear_buffers();
            void commence_buffer_collection();
            void process_instructions();
            void process_a_buffer();

        public:
            // Derived properties
            float bytes_per_sample();
            U32 bytes_per_buffer();
            INT64 samples_per_acquisition();
            U32 buffers_per_acquisition();

    };

    inline uint32_t ats9462_digitizer::reference_source_to_uint( reference_source_t a_reference_source )
    {
        return static_cast< uint32_t >( a_reference_source );
    }
    inline ats9462_digitizer::reference_source_t ats9462_digitizer::uint_to_reference_source( uint32_t a_reference_source_uint )
    {
        return static_cast< ats9462_digitizer::reference_source_t >( a_reference_source_uint );
    }
    inline void ats9462_digitizer::set_reference_source_and_decimation( const std::string& a_reference_source, U32 a_decimation_factor )
    {
        set_reference_source_and_decimation( string_to_reference_source( a_reference_source ), a_decimation_factor );
    }
    inline std::string ats9462_digitizer::get_reference_source_str() const
    {
        return reference_source_to_string( f_reference_source );
    }

    class ats9462_digitizer_binding : public sandfly::_node_binding< ats9462_digitizer, ats9462_digitizer_binding >
    {
        public:
            ats9462_digitizer_binding();
            virtual ~ats9462_digitizer_binding();

        private:
            virtual void do_apply_config(ats9462_digitizer* a_node, const scarab::param_node& a_config ) const;
            virtual void do_dump_config( const ats9462_digitizer* a_node, scarab::param_node& a_config ) const;
    };

    // ATS-specific exceptions for handling in try/catch blocks
    class buffer_overflow : public fast_daq::error
    {
        public:
            buffer_overflow() { f_message = "ATS9462 has overflown the output buffer FIFO"; }
            virtual ~buffer_overflow() {}
    };

} /* namespace fast_daq */
#endif /* ATS9462_WRAP_HH_ */

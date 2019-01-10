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
//TODO (maybe some/all of these could go in the source file?)
#include "AlazarError.h"
#include "AlazarApi.h"
#include "AlazarCmd.h"

// psyllid includes
//#include "memory_block.hh"
#include "node_builder.hh"

#include "producer.hh"
#include "control_access.hh"


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

     Output Streams
     - 0: real_time_data

    */
    class ats9462_digitizer : public midge::_producer< midge::type_list< real_time_data > >, public psyllid::control_access
    {
        private:
            typedef boost::bimap< uint32_t, ALAZAR_SAMPLE_RATES > sample_rate_code_map_t;
            typedef sample_rate_code_map_t::value_type rate_mapping_t;

        public:
            ats9462_digitizer();
            virtual ~ats9462_digitizer();

        private:
            void set_internal_maps();

        public: //node API
            virtual void initialize();
            virtual void execute( midge::diptera* a_midge = nullptr );
            virtual void finalize();

        //TODO implement custom setters that do not allow changes after the board has been configured (for those parameters set in board configuration)
        mv_accessible( U32, samples_per_sec );
        mv_accessible( double, acquisition_length_sec );
        mv_accessible( U32, samples_per_buffer );
        mv_accessible( U32, dma_buffer_count );
        mv_accessible( U32, system_id );
        mv_accessible( U32, board_id );
        mv_accessible( uint64_t, out_length );
        mv_accessible( double, trigger_delay_sec );
        mv_accessible( double, trigger_timeout_sec );

        private:
            sample_rate_code_map_t f_sample_rate_to_code;
            U8 f_channel_count;
            U32 f_channel_mask;
            U8 f_bits_per_sample;
            U32 f_max_samples_per_channel;
            HANDLE f_board_handle;
            bool f_paused;
            std::vector<U16*> f_board_buffers;
            U32 f_buffers_completed;

        private:
            bool check_return_code(RETURN_CODE a_return_code, std::string an_action, unsigned to_throw);
            void configure_board();
            void allocate_buffers();
            void clear_buffers();
            void process_instructions();
            void process_a_buffer();

        public:
            // Derived properties
            float bytes_per_sample();
            U32 bytes_per_buffer();
            INT64 samples_per_acquisition();
            U32 buffers_per_acquisition();

    };

    class ats9462_digitizer_binding : public psyllid::_node_binding< ats9462_digitizer, ats9462_digitizer_binding >
    {
        public:
            ats9462_digitizer_binding();
            virtual ~ats9462_digitizer_binding();

        private:
            virtual void do_apply_config(ats9462_digitizer* a_node, const scarab::param_node& a_config ) const;
            virtual void do_dump_config( const ats9462_digitizer* a_node, scarab::param_node& a_config ) const;
    };
} /* namespace fast_daq */
#endif /* ATS9462_WRAP_HH_ */
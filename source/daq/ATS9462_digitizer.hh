/*
 * ATS9464_digitizer.hh
 *
 * Created on: Nov. 28, 2018
 *     Author: laroque
 */

#ifndef ATS9462_WRAP_HH_
#define ATS9462_WRAP_HH_

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

     At least for this first pass, the digitizer will only support a single channel

     Node type: "ats9462"

     Available configuration values:
     - "samples-per-buffer": int -- number of real-valued samples to include in each chunk of data
     - "out-length": int -- number of output buffer slots
     - "dma-buffer-count": int -- the number of DMA buffers to use between the digitzer board and the application

     Output Streams
     - 0: real_time_data
    */
    //class ats9462_digitizer : public midge::_producer< ats9462_digitizer, typelist_1( psyllid::time_data ) >, public psyllid::control_access
    class ats9462_digitizer : public midge::_producer< ats9462_digitizer, typelist_1( real_time_data ) >, public psyllid::control_access
    {
        public:
            ats9462_digitizer();
            virtual ~ats9462_digitizer();

        public: //node API
            virtual void initialize();
            virtual void execute( midge::diptera* a_midge = nullptr );
            virtual void finalize();

        mv_accessible( double, samples_per_sec ); //TODO this float is only partly used, the rate has to be from the ALAZAR_SAMPLE_RATES enum in AlazarCmd.h ... should deal with this more carefully.
        mv_accessible( double, acquisition_length_sec );
        mv_accessible( U32, samples_per_buffer );
        mv_accessible( U32, dma_buffer_count );
        mv_accessible( U32, system_id );
        mv_accessible( U32, board_id );
        mv_accessible( uint64_t, out_length );
        mv_accessible( double, trigger_delay_sec );
        mv_accessible( double, trigger_timeout_sec );

        private:
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

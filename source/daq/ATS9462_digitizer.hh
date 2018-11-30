/*
 * ATS9464.hh
 *
 * Created on: Nov. 28, 2018
 *     Author: laroque
 */

#ifndef ATS9462_WRAP_HH_
#define ATS9462_WRAP_HH_

// AlazarTech includes
#include "AlazarError.h"
#include "AlazarApi.h"
#include "AlazarCmd.h"

// psyllid includes
#include "memory_block.hh"
#include "node_builder.hh"

#include "producer.hh"
#include "control_access.hh"

// forward declarations
namespace psyllid
{
    class time_data;
}


namespace fast_daq
{
    /*!
     @class ats9462_digitizer
     @author B. H. LaRoque

     @brief A producer to read time-domain data from an AlazarTech ATS9462 digitizer

     @details

     At least for this first pass, the digitizer will only support a single channel

     Node type: "ats9462"

     Available configuration values:
     - "samples-per-buffer": int -- number of real-valued samples to include in each chunk of data

     Output Streams
     - 0: time_data
    */
    class ats9462_digitizer : public midge::_producer< ats9462_digitizer, typelist_1( psyllid::time_data ) >
    {
        public:
            ats9462_digitizer();
            virtual ~ats9462_digitizer();

        public: //node API
            virtual void initialize();
            virtual void execute( midge::diptera* a_midge = nullptr );
            virtual void finalize();

        private:
            // These should use the scarab mv_* macros once I migrate
            double samples_per_sec;
            double acquisition_length_sec;
            U32 samples_per_buffer;
            U32 dma_buffer_count;
            U32 system_id;
            U32 board_id;
            U8 channel_count;
            U8 bits_per_sample;
            U32 max_samples_per_channel;
        private:
            HANDLE board;

        private:
            void configure_board();

        public:
            // Derived properties
            float bytes_per_sample();
            U32 bytes_per_buffer();
            INT64 samples_per_acquisition();
            U32 buffers_per_acquisition();

    };

    class ats9462_digitizer_binding : public psyllid::_node_binding< ats9462_digitizer, ats9462_digitizer_binding >
    //class ats9462_digitizer_binding : public _node_binding< ats9462_digitizer, ats9462_digitizer_binding >
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

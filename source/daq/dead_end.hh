/*
 * ead_end.hh
 *
 * Created on: Dec. 7, 2018
 *     Author: laroque
 */

#ifndef DEAD_END_HH_
#define DEAD_END_HH_

// psyllid includes
//#include "memory_block.hh"
#include "node_builder.hh"

//#include "control_access.hh"
#include "consumer.hh"
#include "shared_cancel.hh"


namespace fast_daq
{
    // forward declarations
    class real_time_data;
    /*!
     @class dead_end
     @author B. H. LaRoque

     @brief A node whih has input streams for a range of data types and does nothing with it.

     @details

     Intended purely for debugging purposes, this class receives data but produces no output.
     You may add further actions to check particular input data, but it should be in a helper function which
     can be disabled (and should be disabled by default).

     Node type: "dead-end"

     Available configuration values:

     Input Streams
     - 0: real_time_data

     Output Streams
        None

    */
    class dead_end : public midge::_consumer< dead_end, typelist_1( real_time_data ) >
    {
        public:
            dead_end();
            virtual ~dead_end();

        public: //node API
            virtual void initialize();
            virtual void execute( midge::diptera* a_midge = nullptr );
            virtual void finalize();

        /*
        mv_accessible( double, samples_per_sec );
        */

        private:
            /*
            U8 f_channel_count;
            */

        /*
        private:
            bool check_return_code(RETURN_CODE a_return_code, std::string an_action, unsigned to_throw);
        */

    };

    class dead_end_binding : public psyllid::_node_binding< dead_end, dead_end_binding >
    {
        public:
            dead_end_binding();
            virtual ~dead_end_binding();

        private:
            virtual void do_apply_config(dead_end* a_node, const scarab::param_node& a_config ) const;
            virtual void do_dump_config( const dead_end* a_node, scarab::param_node& a_config ) const;
    };
} /* namespace fast_daq */
#endif /* DEAD_END_HH_ */

/*
 * data_producer.hh
 *
 *  Created on: May 31, 2019
 *      Author: N.S. Oblath
 */

#ifndef DATA_PRODUCER_HH_
#define DATA_PRODUCER_HH_

#include "real_time_data.hh"

#include "node_builder.hh"

#include "producer.hh"

namespace fast_daq
{

    /*!
     @class data_producer
     @author N. S. Oblath

     @brief A producer to use for debugging: continously outputss identical blank data

     @details

     The data are all output as `memory_block` objects.
     However the underlying data that is output is time_data.

     Parameter setting is not thread-safe.  Executing is thread-safe.

     Node type: "data-producer"

     Available configuration values:
     - "length": uint -- The size of the output data buffer
     - "data-size": uint -- The size of the output data objects

     Output Stream:
     - 0: memory_block

    */
    class data_producer : public midge::_producer< midge::type_list< real_time_data > >
    {
        public:
            data_producer();
            virtual ~data_producer();

            mv_accessible( uint64_t, length );
            mv_accessible( uint32_t, data_size );
            mv_accessible( uint16_t, data_value );
            mv_accessible( double, dynamic_range );

            mv_referrable( real_time_data, primary_packet );

            void write_primary_packet();

        public:
            virtual void initialize();
            virtual void execute( midge::diptera* a_midge = nullptr );
            virtual void finalize();

        private:
            void initialize_block( real_time_data* a_block );

    };

    class data_producer_binding : public sandfly::_node_binding< data_producer, data_producer_binding >
    {
        public:
            data_producer_binding();
            virtual ~data_producer_binding();

        private:
            virtual void do_apply_config( data_producer* a_node, const scarab::param_node& a_config ) const;
            virtual void do_dump_config( const data_producer* a_node, scarab::param_node& a_config ) const;
    };

} /* namespace fast_daq */

#endif /* DATA_PRODUCER_HH_ */

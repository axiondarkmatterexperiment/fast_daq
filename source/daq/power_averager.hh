/*
 * power_averager.hh
 *
 * Created on: Dec. 17, 2018
 *     Author: laroque
 */

#ifndef POWER_AVERAGER_HH_
#define POWER_AVERAGER_HH_

// psyllid includes
//#include "memory_block.hh"
#include "node_builder.hh"

//#include "control_access.hh"
#include "transformer.hh"
#include "shared_cancel.hh"


namespace fast_daq
{
    // forward declarations
    class real_time_data;
    class frequency_data;
    class power_data;
    /*!
     @class power_averager
     @author B. H. LaRoque

     @brief A node to incoherently average power spectra

     @details

     Collects a configurable number of power spectra inputs (as complex voltages) and computes
     an incoherent average of power. If an acquisition ends before the configured number of elements
     is received, the average as-collected average is sent (re-weighted for the number of terms collected...
     because the average is computed by scaling each term before adding to the sum, because that's
     generally more reliable when using finite precision).

     Node type: "power-averager"

     Available configuration values:
     - num-output-buffers: (int) -- number of output buffer slots (default==5)
     - spectrum-size: (int) -- number of bins in the output spectrum
     - num-to-average: (int) -- number of buffers to average together

     Input Streams
     - 1: frequency_data

    Output Streams
    - 0: power_data

    */
    class power_averager : public midge::_transformer< midge::type_list<  frequency_data >, midge::type_list< power_data > >
    {
        public:
            power_averager();
            virtual ~power_averager();

        public: //node API
            virtual void initialize();
            virtual void execute( midge::diptera* a_midge = nullptr );
            virtual void finalize();

        private:
            void handle_start();
            void handle_run();
            void handle_stop();
            void send_output();

        mv_accessible( unsigned, num_output_buffers );
        mv_accessible( unsigned, spectrum_size );
        mv_accessible( unsigned, num_to_average );
        private:
            std::vector< double > f_average_spectrum;
            unsigned f_input_counter;

    };

    class power_averager_binding : public psyllid::_node_binding< power_averager, power_averager_binding >
    {
        public:
            power_averager_binding();
            virtual ~power_averager_binding();

        private:
            virtual void do_apply_config(power_averager* a_node, const scarab::param_node& a_config ) const;
            virtual void do_dump_config( const power_averager* a_node, scarab::param_node& a_config ) const;
    };
} /* namespace fast_daq */
#endif /* POWER_AVERAGER_HH_ */

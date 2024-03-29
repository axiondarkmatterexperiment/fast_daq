/*
 * spectrum_relay.hh
 *
 * Created on: Dec. 18, 2018
 *     Author: laroque
 */

#ifndef SPECTRUM_RELAY_HH_
#define SPECTRUM_RELAY_HH_

// sandfly includes
#include "node_builder.hh"

#include "control_access.hh"
#include "consumer.hh"
#include "shared_cancel.hh"


namespace fast_daq
{
    // forward declarations
    class power_data;

    /*!
     @class spectrum_relay
     @author B. H. LaRoque

     @brief A node which receives power spectrum data and broadcasts it via dripline message.

     @details

     Medium resolution data are expected to be received (for example, from a power-averager node)
     to be sent out in a slack message, with proper associated metadata. This can be handled however
     we like, but most probably it is to be logged in a (postgreSQL) database.

     Node type: "spectrum-relay"

     Available configuration values:
     - "spectrum-alert-rk": string -- A valid AMQP routing key to which each medium-res spectrum will be broadcast

     Input Streams
     - 1: power_data

    */
    class spectrum_relay : public midge::_consumer< midge::type_list< power_data > >, public sandfly::control_access
    {
        public:
            spectrum_relay();
            virtual ~spectrum_relay();

        mv_accessible( std::string, spectrum_alert_rk );

        public: //node API
            virtual void initialize();
            virtual void execute( midge::diptera* a_midge = nullptr );
            virtual void finalize();

        private:
            std::thread f_dl_relay_thread;

        private:
            void broadcast_spectrum( power_data* a_spectrum );
    };

    class spectrum_relay_binding : public sandfly::_node_binding< spectrum_relay, spectrum_relay_binding >
    {
        public:
            spectrum_relay_binding();
            virtual ~spectrum_relay_binding();

        private:
            virtual void do_apply_config(spectrum_relay* a_node, const scarab::param_node& a_config ) const;
            virtual void do_dump_config( const spectrum_relay* a_node, scarab::param_node& a_config ) const;
    };
} /* namespace fast_daq */
#endif /* SPECTRUM_RELAY */

/*
 * fast_daq_relayer.hh
 *
 *  Created on: Jan 29, 2026
 *      Author: N.S. Oblath
 */

#ifndef FAST_DAQ_RELAYER_HH_
#define FAST_DAQ_RELAYER_HH_

#include "message_relayer.hh"


namespace scarab
{
    class param_node;
}

namespace fast_daq
{

    class fast_daq_relayer : public sandfly::message_relayer
    {
        public:
            fast_daq_relayer( const scarab::param_node& a_config, const scarab::authentication& a_auth );
            fast_daq_relayer( const fast_daq_relayer& ) = delete;
            fast_daq_relayer( fast_daq_relayer&& ) = default;
            virtual ~fast_daq_relayer() = default;

            fast_daq_relayer& operator=( const fast_daq_relayer& ) = delete;
            fast_daq_relayer& operator=( fast_daq_relayer&& ) = default;

        public:
            void send_spectrum( const scarab::param_ptr_t&& a_payload ) const;

            void send_notice( const std::string& a_msg_text ) const override;
            void send_warn( const std::string& a_msg_text ) const override;
            void send_error( const std::string& a_msg_text ) const override;
            void send_critical( const std::string& a_msg_text ) const override;

            void send_notice( scarab::param_ptr_t&& a_payload ) const override;
            void send_warn( scarab::param_ptr_t&& a_payload ) const override;
            void send_error( scarab::param_ptr_t&& a_payload ) const override;
            void send_critical( scarab::param_ptr_t&& a_payload ) const override;
    };

} /* namespace fast_daq */

#endif /* FAST_DAQ_RELAYER_HH_ */

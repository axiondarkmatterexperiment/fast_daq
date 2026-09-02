/*
 * fast_daq_relayer.cc
 *
 *  Created on: Jan 29, 2026
 *      Author: N.S. Oblath
 */

#include "fast_daq_relayer.hh"

#include "authentication.hh"
#include "param.hh"
#include "param_helpers_impl.hh"

using scarab::param_node;
using_param_args_and_kwargs;

namespace fast_daq
{

    fast_daq_relayer::fast_daq_relayer( const scarab::param_node& a_config, const scarab::authentication& a_auth ) :
            sandfly::message_relayer( a_config, a_auth )
    {}

    void fast_daq_relayer::send_spectrum( const scarab::param_ptr_t&& a_payload ) const
    {
        // TODO: Implement send-spectrum functionality
        return;
    }

    void fast_daq_relayer::send_notice( const std::string& a_msg_text ) const
    {
        return;
    }

    void fast_daq_relayer::send_warn( const std::string& a_msg_text ) const
    {
        return;
    }

    void fast_daq_relayer::send_error( const std::string& a_msg_text ) const
    {
        return;
    }

    void fast_daq_relayer::send_critical( const std::string& a_msg_text ) const
    {
        return;
    }

    void fast_daq_relayer::send_notice( scarab::param_ptr_t&& a_payload ) const
    {
        return;
    }

    void fast_daq_relayer::send_warn( scarab::param_ptr_t&& a_payload ) const
    {
        return;
    }

    void fast_daq_relayer::send_error( scarab::param_ptr_t&& a_payload ) const
    {
        return;
    }

    void fast_daq_relayer::send_critical( scarab::param_ptr_t&& a_payload ) const
    {
        return;
    }
} /* namespace fast_daq */

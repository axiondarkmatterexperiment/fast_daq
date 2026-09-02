/*
 * fast_daq_config.cc
 *
 *  Created on: Jan 30, 2026
 *      Author: N.S. Oblath
 */

#include "fast_daq_config.hh"
#include "logger.hh"
#include "sandfly_error.hh"

// dripline
#include "dripline_config.hh"

//scarab
#include "application.hh"
#include "path.hh"

#include<string>

using std::string;

using scarab::param_array;
using scarab::param_node;
using scarab::param_value;

namespace fast_daq
{

    LOGGER( plog, "fast_daq_config" );

    fast_daq_config::fast_daq_config() : server_config()
    {
        // default fast_daq configuration
        (*this)["name"]() = "fast_daq";
        (*this)["use-relayer"]() = false;
        (*this)["control"]["subrun-duration-ms" ]() = 100U;
    }

    fast_daq_config::~fast_daq_config()
    {
    }


    void add_fast_daq_options( scarab::main_app& an_app )
    {
        sandfly::add_sandfly_options( an_app );

        return;
    }


} /* namespace fast_daq */

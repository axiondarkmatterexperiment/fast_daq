/*
 * fast_daq_config.hh
 *
 *  Created on: Jan 30, 2026
 *      Author: N.S. Oblath
 */

#ifndef FAST_DAQ_CONFIG_CONFIG_HH_
#define FAST_DAQ_CONFIG_CONFIG_HH_

#include "server_config.hh"

//namespace scarab
//{
//    class main_app;
//}

namespace fast_daq
{
    /*!
     @class fast_daq_config
     @author N. S. Oblath

     @brief Contains default fast_daq configuration

     @details
     Contains default configurations for:
     - dripline_mesh
     - activate-at-startup
     - n-files
     - duration
     - use-relayer
     - max-file-size-mb

     These default configurations, together with the configurations from the command line and the config-file, are passed to scarab::configurator by the fast_daq executable.
     The configurator combines them and extracts the final fast_daq configuration which is then passed to the run_server during initialization.
     */
    class fast_daq_config : public sandfly::server_config
    {
        public:
            fast_daq_config();
            virtual ~fast_daq_config();
    };

    /// Add basic fast_daq options to an app object
    void add_fast_daq_options( scarab::main_app& an_app );

} /* namespace fast_daq */

#endif /* FAST_DAQ_CONFIG_CONFIG_HH_ */

/*
 * fast_daq_version.hh
 *
 *  Created on: Mar 20, 2013
 *      Author: nsoblath
 */

#ifndef FAST_DAQ_VERSION_HH_
#define FAST_DAQ_VERSION_HH_

#include "dripline_version.hh"

namespace fast_daq
{
    class version : public scarab::version_semantic
    {
        public:
            version();
            ~version();
    };

} // namespace fast_daq

#endif /* FAST_DAQ_VERSION_HH_ */

/*
 * psyllid.cc
 *
 *  Created on: Feb 1, 2016
 *      Author: N.S. Oblath
 */

#include "daq_control.hh"
#include "fast_daq_error.hh"
#include "fast_daq_version.hh"

#include "conductor.hh"
#include "sandfly_error.hh"
#include "server_config.hh"

#include "application.hh"
#include "logger.hh"

using namespace psyllid;
using namespace sandfly;

using std::string;

LOGGER( plog, "fast_daq" );

int main( int argc, char** argv )
{
    LINFO( plog, "Welcome to Fast Daq\n\n" <<
      "\t\t        _____     _      ____     _____        ____       _       ___       \n" <<
      "\t\t         |" ___|U  /"\  u / __"| u |_ " _|      |  _"\  U  /"\  u  / " \    \n" <<    
      "\t\t         U| |_  u \/ _ \/ <\___ \/    | |       /| | | |  \/ _ \/  | |"| |  \n" <<     
      "\t\t         \|  _|/  / ___ \  u___) |   /| |\      U| |_| |\ / ___ \ /| |_| |\ \n" <<     
      "\t\t   |_|    /_/   \_\ |____/>> u |_|U       |____/ u/_/   \_\U \__\_\u        \n" <<
      "\t\t    )(\\,-  \\    >>  )(  (__)_// \\_       |||_    \\    >>   \\//         \n" <<
      "\t\t        (__)(_/ (__)  (__)(__)    (__) (__)     (__)_)  (__)  (__) (_(__)   \n\n");
    try
    {
        // The application
        scarab::main_app the_main;
        conductor the_conductor;
        the_conductor.set_rc_creator< daq_control >();

        // Default configuration
        the_main.default_config() = server_config();

        // The main execution callback
        the_main.callback( [&](){ the_conductor.execute( the_main.master_config() ); } );

        // Command line options
        add_sandfly_options( the_main );

        // Package version
        the_main.set_version( std::make_shared< psyllid::version >() );

        // Parse CL options and run the application
        CLI11_PARSE( the_main, argc, argv );

        return the_conductor.get_return();

    }
    catch( scarab::error& e )
    {
        LERROR( plog, "configuration error: " << e.what() );
        return RETURN_ERROR;
    }
    catch( psyllid::error& e )
    {
        LERROR( plog, "psyllid error: " << e.what() );
        return RETURN_ERROR;
    }
    catch( sandfly::error& e )
    {
        LERROR( plog, "sandfly error: " << e.what() );
        return RETURN_ERROR;
    }
    catch( std::exception& e )
    {
        LERROR( plog, "std::exception caught: " << e.what() );
        return RETURN_ERROR;
    }
    catch( ... )
    {
        LERROR( plog, "unknown exception caught" );
        return RETURN_ERROR;
    }

    return RETURN_ERROR;
}

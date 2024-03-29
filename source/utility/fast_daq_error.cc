#include <iostream>

#include "fast_daq_error.hh"

namespace fast_daq
{

    error::error() :
            exception(),
            f_message( "" )
    {
    }
    error::error( const error& p_copy ) :
            exception( p_copy ),
            f_message( p_copy.f_message )
    {
    }
    error& error::operator=( const error& p_copy )
    {
        exception::operator=( p_copy );
        f_message = p_copy.f_message;
        return *this;
    }
    error::~error() throw()
    {
    }

    const char* error::what() const throw()
    {
        return f_message.c_str();
    }

}


#ifndef _fast_daq_error_hh_
#define _fast_daq_error_hh_

#include <exception>
#include <string>
#include <sstream>

namespace fast_daq
{

    class error : public std::exception
    {
        public:
            error();
            error( const error& p_copy );
            error& operator=( const error& p_copy );
            virtual ~error() throw ();

            template< class x_type >
            error& operator<<( const x_type& p_fragment );

            const char* what() const throw ();

        protected:
            std::string f_message;
    };

    template< class x_type >
    error& error::operator<<( const x_type& p_fragment )
    {
        std::stringstream f_converter;
        f_converter << f_message << p_fragment;
        f_message.assign( f_converter.str() );
        return (*this);
    }

}

#endif

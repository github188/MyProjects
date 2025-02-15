//  Boost system_error.hpp  --------------------------------------------------//

//  Copyright Beman Dawes 2006

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_SYSTEM_ERROR_HPP
#define BOOST_SYSTEM_ERROR_HPP

#include <string>
#include <stdexcept>
#include <cassert>
#include <boost/system/error_code.hpp>

namespace boost
{
  namespace system
  {
    //  class system_error  --------------------------------------------------//

    class system_error : public std::runtime_error
    {
    public:
      system_error( error_code ec )
          : std::runtime_error(""), m_error_code(ec) {}

      system_error( error_code ec, const std::string & what_arg )
          : std::runtime_error(what_arg), m_error_code(ec) {}

      system_error( error_code ec, const char* what_arg )
          : std::runtime_error(what_arg), m_error_code(ec) {}

      system_error( int ev, const error_category & ecat )
          : std::runtime_error(""), m_error_code(ev,ecat) {}

      system_error( int ev, const error_category & ecat,
        const std::string & what_arg )
          : std::runtime_error(what_arg), m_error_code(ev,ecat) {}

      system_error( int ev, const error_category & ecat,
        const char * what_arg )
          : std::runtime_error(what_arg), m_error_code(ev,ecat) {}

      virtual ~system_error() throw() {}

      const error_code &  code() const throw() { return m_error_code; }
      const char *        what() const throw();

    private:
      error_code           m_error_code;
      mutable std::string  m_what;
    };

    //  implementation  ------------------------------------------------------//

    inline const char * system_error::what() const throw()
    // see http://www.boost.org/more/error_handling.html for lazy build rationale
    {
        typedef std::runtime_error super;
      if ( m_what.empty() )
      {
        try
        {
          m_what = super::what();
          if ( m_error_code )
          {
            if ( !m_what.empty() ) m_what += ": ";
            m_what += m_error_code.message();
          }
        }
        catch (...) { return super::what(); }
      }
      return m_what.c_str();
    }

  } // namespace system
} // namespace boost

#endif // BOOST_SYSTEM_ERROR_HPP



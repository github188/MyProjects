//
// socket_types.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_SOCKET_TYPES_HPP
#define BOOST_ASIO_DETAIL_SOCKET_TYPES_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <boost/config.hpp>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
# if defined(_WINSOCKAPI_) && !defined(_WINSOCK2API_)
#  error WinSock.h has already been included
# endif // defined(_WINSOCKAPI_) && !defined(_WINSOCK2API_)
# if !defined(_WIN32_WINNT) && !defined(_WIN32_WINDOWS)
#  if defined(_MSC_VER) || defined(__BORLANDC__)
#   pragma message("Please define _WIN32_WINNT or _WIN32_WINDOWS appropriately")
#   pragma message("Assuming _WIN32_WINNT=0x0501 (i.e. Windows XP target)")
#  else // defined(_MSC_VER) || defined(__BORLANDC__)
#   warning Please define _WIN32_WINNT or _WIN32_WINDOWS appropriately
#   warning Assuming _WIN32_WINNT=0x0501 (i.e. Windows XP target)
#  endif // defined(_MSC_VER) || defined(__BORLANDC__)
#  define _WIN32_WINNT 0x0501
# endif // !defined(_WIN32_WINNT) && !defined(_WIN32_WINDOWS)
# if defined(_MSC_VER)
#  if defined(_WIN32) && !defined(WIN32)
#   if !defined(_WINSOCK2API_)
#    define WIN32 // Needed for correct types in winsock2.h
#   else // !defined(_WINSOCK2API_)
#    error Please define the macro WIN32 in your compiler options
#   endif // !defined(_WINSOCK2API_)
#  endif // defined(_WIN32) && !defined(WIN32)
# endif // defined(_MSC_VER)
# if defined(__BORLANDC__)
#  include <stdlib.h> // Needed for __errno
#  if defined(__WIN32__) && !defined(WIN32)
#   if !defined(_WINSOCK2API_)
#    define WIN32 // Needed for correct types in winsock2.h
#   else // !defined(_WINSOCK2API_)
#    error Please define the macro WIN32 in your compiler options
#   endif // !defined(_WINSOCK2API_)
#  endif // defined(__WIN32__) && !defined(WIN32)
#  if !defined(_WSPIAPI_H_)
#   define _WSPIAPI_H_
#   define BOOST_ASIO_WSPIAPI_H_DEFINED
#  endif // !defined(_WSPIAPI_H_)
# endif // defined(__BORLANDC__)
# if !defined(BOOST_ASIO_NO_WIN32_LEAN_AND_MEAN)
#  if !defined(WIN32_LEAN_AND_MEAN)
#   define WIN32_LEAN_AND_MEAN
#  endif // !defined(WIN32_LEAN_AND_MEAN)
# endif // !defined(BOOST_ASIO_NO_WIN32_LEAN_AND_MEAN)
# if defined(__CYGWIN__)
#  if !defined(__USE_W32_SOCKETS)
#   error You must add -D__USE_W32_SOCKETS to your compiler options.
#  endif // !defined(__USE_W32_SOCKETS)
#  if !defined(NOMINMAX)
#   define NOMINMAX 1
#  endif // !defined(NOMINMAX)
# endif // defined(__CYGWIN__)
# include <winsock2.h>
# include <ws2tcpip.h>
# include <mswsock.h>
# if defined(BOOST_ASIO_WSPIAPI_H_DEFINED)
#  undef _WSPIAPI_H_
#  undef BOOST_ASIO_WSPIAPI_H_DEFINED
# endif // defined(BOOST_ASIO_WSPIAPI_H_DEFINED)
# if !defined(BOOST_ASIO_NO_DEFAULT_LINKED_LIBS)
#  if defined(UNDER_CE)
#   pragma comment(lib, "ws2.lib")
#  elif defined(WIN_PHONE)
#   pragma comment(lib, "ws2_32.lib")
#  elif defined(_MSC_VER) || defined(__BORLANDC__)
#   pragma comment(lib, "ws2_32.lib")
#   pragma comment(lib, "mswsock.lib")
#  endif // defined(_MSC_VER) || defined(__BORLANDC__)
# endif // !defined(BOOST_ASIO_NO_DEFAULT_LINKED_LIBS)
# include <boost/asio/detail/old_win_sdk_compat.hpp>
#else
# include <sys/ioctl.h>
# include <sys/poll.h>
# include <sys/types.h>
# if defined(__hpux) && !defined(__HP_aCC)
#  include <sys/time.h>
# else
#  include <sys/select.h>
# endif
# include <sys/socket.h>
# include <sys/uio.h>
# include <sys/un.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <net/if.h>
# include <limits.h>
# if defined(__sun)
#  include <sys/filio.h>
#  include <sys/sockio.h>
# endif
#endif
#include <boost/asio/detail/pop_options.hpp>

namespace boost {
namespace asio {
namespace detail {

#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
typedef SOCKET socket_type;
const SOCKET invalid_socket = INVALID_SOCKET;
const int socket_error_retval = SOCKET_ERROR;
const int max_addr_v4_str_len = 256;
const int max_addr_v6_str_len = 256;
typedef sockaddr socket_addr_type;
typedef in_addr in4_addr_type;
typedef ip_mreq in4_mreq_type;
typedef sockaddr_in sockaddr_in4_type;
# if defined(BOOST_ASIO_HAS_OLD_WIN_SDK)
typedef in6_addr_emulation in6_addr_type;
typedef ipv6_mreq_emulation in6_mreq_type;
typedef sockaddr_in6_emulation sockaddr_in6_type;
typedef sockaddr_storage_emulation sockaddr_storage_type;
typedef addrinfo_emulation addrinfo_type;
# else
typedef in6_addr in6_addr_type;
typedef ipv6_mreq in6_mreq_type;
typedef sockaddr_in6 sockaddr_in6_type;
typedef sockaddr_storage sockaddr_storage_type;
typedef addrinfo addrinfo_type;
# endif
typedef unsigned long ioctl_arg_type;
typedef u_long u_long_type;
typedef u_short u_short_type;
const int shutdown_receive = SD_RECEIVE;
const int shutdown_send = SD_SEND;
const int shutdown_both = SD_BOTH;
const int message_peek = MSG_PEEK;
const int message_out_of_band = MSG_OOB;
const int message_do_not_route = MSG_DONTROUTE;
# if defined (_WIN32_WINNT)
const int max_iov_len = 64;
# else
const int max_iov_len = 16;
# endif
#else
typedef int socket_type;
const int invalid_socket = -1;
const int socket_error_retval = -1;
const int max_addr_v4_str_len = INET_ADDRSTRLEN;
const int max_addr_v6_str_len = INET6_ADDRSTRLEN + 1 + IF_NAMESIZE;
typedef sockaddr socket_addr_type;
typedef in_addr in4_addr_type;
# if defined(__hpux)
// HP-UX doesn't provide ip_mreq when _XOPEN_SOURCE_EXTENDED is defined.
struct in4_mreq_type
{
  struct in_addr imr_multiaddr;
  struct in_addr imr_interface;
};
# else
typedef ip_mreq in4_mreq_type;
# endif
typedef sockaddr_in sockaddr_in4_type;
typedef in6_addr in6_addr_type;
typedef ipv6_mreq in6_mreq_type;
typedef sockaddr_in6 sockaddr_in6_type;
typedef sockaddr_storage sockaddr_storage_type;
typedef sockaddr_un sockaddr_un_type;
typedef addrinfo addrinfo_type;
typedef int ioctl_arg_type;
typedef uint32_t u_long_type;
typedef uint16_t u_short_type;
const int shutdown_receive = SHUT_RD;
const int shutdown_send = SHUT_WR;
const int shutdown_both = SHUT_RDWR;
const int message_peek = MSG_PEEK;
const int message_out_of_band = MSG_OOB;
const int message_do_not_route = MSG_DONTROUTE;
# if defined(IOV_MAX)
const int max_iov_len = IOV_MAX;
# else
// POSIX platforms are not required to define IOV_MAX.
const int max_iov_len = 16;
# endif
#endif
const int custom_socket_option_level = 0xA5100000;
const int enable_connection_aborted_option = 1;
const int always_fail_option = 2;

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_SOCKET_TYPES_HPP

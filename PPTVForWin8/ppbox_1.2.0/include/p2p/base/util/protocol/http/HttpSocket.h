// HttpSocket.h

#ifndef _UTIL_PROTOCOL_HTTP_SOCKET_H_
#define _UTIL_PROTOCOL_HTTP_SOCKET_H_

#include "util/protocol/http/HttpHead.h"

#include <framework/network/TcpSocket.h>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>

namespace util
{
    namespace protocol
    {

        class HttpPacket;

        namespace detail
        {

            template <typename Handler>
            struct receive_handler
            {
                receive_handler(
                    HttpHead & head, 
                    boost::asio::streambuf & buf, 
                    Handler handler)
                    : head_(head)
                    , buf_(buf)
                    , handler_(handler)
                {
                }

                void operator()(
                    boost::system::error_code const & ec, 
                    size_t bytes_transferred)
                {
                    if (!ec) {
                        size_t old_size = buf_.size();
                        std::istream is(&buf_);
                        boost::system::error_code ec1;
                        head_.set_content(is, ec1);
                        handler_(ec1, old_size - buf_.size());
                    } else {
                        handler_(ec, 0);
                    }
                }

            //private:
                HttpHead & head_;
                boost::asio::streambuf & buf_;
                Handler handler_;
            };

            template <typename Handler>
            inline void* asio_handler_allocate(
                std::size_t size,
                receive_handler<Handler> * this_handler)
            {
                return boost_asio_handler_alloc_helpers::allocate(
                    size, &this_handler->handler_);
            }

            template <typename Handler>
            inline void asio_handler_deallocate(
                void * pointer, 
                std::size_t size,
                receive_handler<Handler> * this_handler)
            {
                boost_asio_handler_alloc_helpers::deallocate(
                    pointer, size, &this_handler->handler_);
            }

            template <typename Function, typename Handler>
            inline void asio_handler_invoke(
                const Function & function,
                receive_handler<Handler> * this_handler)
            {
                boost_asio_handler_invoke_helpers::invoke(
                    function, &this_handler->handler_);
            }

        } // namespace detail

        class HttpSocket
            : public framework::network::TcpSocket
        {
        public:
            typedef framework::network::TcpSocket super;

        public:
            HttpSocket(
                boost::asio::io_service & io_svc)
                : super(io_svc)
                , non_block_(false)
            {
            }

        public:
            void close()
            {
                buf_.reset();
                super::close();
            }

            boost::system::error_code close(
                boost::system::error_code & ec)
            {
                buf_.reset();
                return super::close(ec);
            }

        public:
            size_t write(
                HttpHead & head)
            {
                if (buf_.size() == 0) {
                    std::ostream os(&buf_);
                    boost::system::error_code ec;
                    head.get_content(os, ec);
                    assert(!ec);
                }
                return boost::asio::write((super &)*this, buf_);
            }

            size_t write(
                HttpHead & head, 
                boost::system::error_code & ec)
            {
                if (buf_.size() == 0) {
                    std::ostream os(&buf_);
                    head.get_content(os, ec);
                    assert(!ec);
                }
                return boost::asio::write((super &)*this, buf_, boost::asio::transfer_all(), ec);
            }

            template <typename Handler>
            void async_write(
                HttpHead & head, 
                Handler const & handler)
            {
                std::ostream os(&buf_);
                boost::system::error_code ec;
                head.get_content(os, ec);
                assert(!ec);
                boost::asio::async_write((super &)*this, buf_, handler);
            }

            size_t read(
                HttpHead & head)
            {
                boost::asio::read_until((super &)*this, buf_, "\r\n\r\n");
                size_t old_size = buf_.size();
                std::istream is(&buf_);
                boost::system::error_code ec;
                head.set_content(is, ec);
                assert(!ec);
                return old_size - buf_.size();
            }

            size_t read(
                HttpHead & head, 
                boost::system::error_code & ec)
            {
                boost::asio::read_until((super &)*this, buf_, "\r\n\r\n", ec);
                if (!ec) {
                    size_t old_size = buf_.size();
                    std::istream is(&buf_);
                    head.set_content(is, ec);
                    return old_size - buf_.size();
                }
                return 0;
            }

            template <typename Handler>
            void async_read(
                HttpHead & head, 
                Handler const & handler)
            {
                boost::asio::async_read_until((super &)*this, buf_, "\r\n\r\n", 
                    detail::receive_handler<Handler>(head, buf_, handler));
            }

        public:
            // ��дreceive��async_receive��read_some��async_read_some
            template <typename MutableBufferSequence>
            std::size_t receive(
                const MutableBufferSequence & buffers)
            {
                using namespace boost::asio;

                if (buf_.size() > 0) {
                    return copy(buffers);
                } else {
                    return super::receive(buffers);
                }
            }

            template <typename MutableBufferSequence>
            std::size_t receive(const MutableBufferSequence& buffers, 
                socket_base::message_flags flags)
            {
                using namespace boost::asio;

                if (buf_.size() > 0) {
                    return copy(buffers);
                } else {
                    return super::receive(buffers, flags);
                }
            }

            template <typename MutableBufferSequence>
            std::size_t receive(
                const MutableBufferSequence& buffers, 
                socket_base::message_flags flags, 
                boost::system::error_code& ec)
            {
                using namespace boost::asio;

                if (buf_.size() > 0) {
                    ec = boost::system::error_code();
                    return copy(buffers);
                } else {
                    return super::receive(buffers, flags, ec);
                }
            }

            template <typename MutableBufferSequence, typename ReadHandler>
            void async_receive(
                const MutableBufferSequence& buffers,
                ReadHandler handler)
            {
                if (buf_.size() > 0) {
                    std::size_t length = copy(buffers);
                    get_io_service().post(boost::asio::detail::bind_handler(
                        handler, boost::system::error_code(), length));
                } else {
                    super::async_receive(buffers, handler);
                }
            }

            template <typename MutableBufferSequence, typename ReadHandler>
            void async_receive(
                const MutableBufferSequence& buffers,
                socket_base::message_flags flags, 
                ReadHandler handler)
            {
                if (buf_.size() > 0) {
                    std::size_t length = copy(buffers);
                    get_io_service().post(boost::asio::detail::bind_handler(
                        handler, boost::system::error_code(), length));
                } else {
                    super::async_receive(buffers, flags, handler);
                }
            }

            template <typename MutableBufferSequence>
            std::size_t read_some(
                const MutableBufferSequence& buffers)
            {
                using namespace boost::asio;

                if (buf_.size() > 0) {
                    return copy(buffers);
                } else {
                    return super::read_some(buffers);
                }
            }

            template <typename MutableBufferSequence>
            std::size_t read_some(
                const MutableBufferSequence& buffers,
                boost::system::error_code& ec)
            {
                using namespace boost::asio;

                if (buf_.size() > 0) {
                    ec = boost::system::error_code();
                    return copy(buffers);
                } else {
                    return super::read_some(buffers, ec);
                }
            }

            template <typename MutableBufferSequence, typename ReadHandler>
            void async_read_some(
                const MutableBufferSequence& buffers,
                ReadHandler handler)
            {
                if (buf_.size() > 0) {
                    std::size_t length = copy(buffers);
                    get_io_service().post(boost::asio::detail::bind_handler(
                        handler, boost::system::error_code(), length));
                } else {
                    super::async_read_some(buffers, handler);
                }
            }

            /// Start an asynchronous read. The buffer into which the data will be read
            /// must be valid for the lifetime of the asynchronous operation.
            template <typename MutableBufferSequence>
            std::size_t copy(
                const MutableBufferSequence& buffers)
            {
                using namespace std; // For memcpy.
                using namespace boost::asio;

                std::size_t bytes_avail = buf_.size();
                std::size_t bytes_copied = 0;

                typename MutableBufferSequence::const_iterator iter = buffers.begin();
                typename MutableBufferSequence::const_iterator end = buffers.end();
                for (; iter != end && bytes_avail > 0; ++iter) {
                    std::size_t max_length = buffer_size(*iter);
                    std::size_t length = (max_length < bytes_avail)
                        ? max_length : bytes_avail;
                    memcpy(buffer_cast<void *>(*iter), buffer_cast<char const *>(buf_.data()) + bytes_copied, length);
                    bytes_copied += length;
                    bytes_avail -= length;
                }

                buf_.consume(bytes_copied);
                return bytes_copied;
            }

        private:
            BOOST_STATIC_CONSTANT(size_t, BUF_SIZE = 2048);

        private:
            bool non_block_;
            boost::asio::streambuf buf_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_HTTP_SOCKET_H_

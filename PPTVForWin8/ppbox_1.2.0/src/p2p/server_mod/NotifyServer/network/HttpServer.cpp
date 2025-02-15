//------------------------------------------------------------------------------------------
//     Copyright (c)2005-2010 PPLive Corporation.  All rights reserved.
//------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "network/HttpServer.h"

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/placeholders.hpp>

#ifdef COUNT_CPU_TIME
#include "count_cpu_time.h"
#endif

#define HTTPSVR_DEBUG(msg) LOG(__DEBUG, "httpserver", __FUNCTION__ << ":" << __LINE__ << ":" << shared_from_this() << " " << msg)

namespace network
{

    FRAMEWORK_LOGGER_DECLARE_MODULE("httpserver");

    class ObjectStates
    {
    public:
        static const uint8_t Alive = 0xBD;
        static const uint8_t Destroyed = 0xAC;
    };

    HttpServer::HttpServer(
        boost::asio::io_service & io_svc)
        : socket_(io_svc)
        , sended_bytes_(0)
        , recv_timer_(global_second_timer(), 60 * 1000, boost::bind(&HttpServer::OnTimerElapsed, this, &recv_timer_))
        , close_timer_(global_second_timer(), 1 * 1000, boost::bind(&HttpServer::OnTimerElapsed, this, &close_timer_))
        , recv_timeout_(60 * 1000)
        , is_open_(false)
        , will_close_(false)
        , object_state_(ObjectStates::Alive)

    {
        is_auto_close_ = true;
    }

    HttpServer::~HttpServer()
    {
        //TODO, ericzheng, remove the dtor & object_state_ member once the crash issue is understood
        if (object_state_ != ObjectStates::Alive)
        {
            //purposely crash the app
            *(reinterpret_cast<uint8_t*>(NULL)) = object_state_;
        }

        object_state_ = ObjectStates::Destroyed;
    }

    void HttpServer::HttpRecv()
    {
        if (is_open_ == false)
            return;

        string delim("\r\n\r\n");
        boost::asio::async_read_until(socket_, request_, delim, boost::bind(&HttpServer::HandleHttpRecv,
            shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

        if (recv_timeout_ > 0)
        {
            recv_timer_.interval(recv_timeout_);
            recv_timer_.start();
        }
    }

    void HttpServer::HttpRecvTillClose()
    {
        if (is_open_ == false)
        {
            HTTPSVR_DEBUG("endpoint: " << socket_.remote_endpoint() << ", is_open = false");
            return;
        }

        protocol::SubPieceBuffer buffer(new protocol::SubPieceContent, 1024);

        HTTPSVR_DEBUG("endpoint: " << socket_.remote_endpoint());

        boost::asio::async_read(socket_, boost::asio::buffer(buffer.Data(), buffer.Length()),
            boost::asio::transfer_at_least(1), boost::bind(&HttpServer::HandleHttpRecvTillClose, shared_from_this(),
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, buffer));
    }

    void HttpServer::HandleHttpRecvTillClose(
        const boost::system::error_code& err,
        uint32_t bytes_transferred,
        protocol::SubPieceBuffer buffer)
    {
        if (is_open_ == false)
        {
            HTTPSVR_DEBUG("is_open = false");
            return;
        }

        if (!err)
        {
            HTTPSVR_DEBUG("Endpoint: " << socket_.remote_endpoint() << ", GetBuffer: size = " << buffer.Length()
                << "Content: \n" << string((char*)buffer.Data(), buffer.Length()));
            HttpRecvTillClose();
        }
        else
        {
            HTTPSVR_DEBUG("err: " << err.message() << ", handler: " << handler_);
            if (handler_)
            {
                handler_->OnHttpRecvFailed(2);
            }

            Close();
        }
    }

    void HttpServer::HandleHttpRecv(const boost::system::error_code& err, uint32_t bytes_transferred)
    {
        if (is_open_ == false)
        {
            HTTPSVR_DEBUG("is_open_ = false, error = " << err.message());
            return;
        }

        recv_timer_.stop();

        if (!err)
        {
            string request_string;
            std::copy(std::istreambuf_iterator<char> (&request_), std::istreambuf_iterator<char> (), std::back_inserter(request_string));

            HTTPSVR_DEBUG("RequestString:\n" << request_string << ", Handler = " << handler_);

            HttpRequest::p http_request = HttpRequest::ParseFromBuffer(request_string);
            if (!http_request)
            {
                HTTPSVR_DEBUG("HttpRequest Parse Error!");
                if (handler_)
                {
                    handler_->OnHttpRecvFailed(1);
                }
            }
            else if (handler_)
            {
                handler_->OnHttpRecvSucced(http_request);
            }
        }
        else
        {
            if (handler_)
            {
                //OutputDebugStringA(err.message().c_str());
                handler_->OnHttpRecvFailed(2);
            }

            Close();
        }
    }

    void HttpServer::OnTimerElapsed(
        framework::timer::Timer * pointer)
    {

        if (false == is_open_)
        {
            HTTPSVR_DEBUG(" is_open = false");
            return;
        }

        if (pointer == &recv_timer_)
        {
            HTTPSVR_DEBUG("timer = recv_timer_");
            HandleHttpRecvTimeout();
        }
        else if (pointer == &close_timer_)
        {
            HTTPSVR_DEBUG("timer = close_timer_");
            HandleCloseTimerElapsed();
        }
    }

    void HttpServer::HandleHttpRecvTimeout()
    {
        if (is_open_ == false)
        {
            HTTPSVR_DEBUG("is_open = false");
            return;
        }

        if (handler_)
        {
            handler_->OnHttpRecvTimeout();
        }
    }

    void HttpServer::TcpSend(const base::AppBuffer& buffer)
    {
#ifdef COUNT_CPU_TIME
        count_cpu_time(__FUNCTION__);
#endif
        if (is_open_ == false)
        {
            HTTPSVR_DEBUG("send_buffer, is_open = false");
            return;
        }

        bool is_send_list_empty = send_list_.empty();
        send_list_.push_back(buffer);

        if (is_send_list_empty)
        {
            boost::asio::async_write(
                socket_,
                boost::asio::buffer(buffer.Data(), buffer.Length()),
                boost::asio::transfer_at_least(buffer.Length()),
                boost::bind(&HttpServer::HandleTcpSend, shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
        }
    }

    void HttpServer::HandleTcpSend(const boost::system::error_code& err, uint32_t bytes_transferred)
    {
#ifdef COUNT_CPU_TIME
        count_cpu_time(__FUNCTION__);
#endif
        if (is_open_ == false)
        {
            return;
        }

        if (!err)
        {
            sended_bytes_ += bytes_transferred;

            HTTPSVR_DEBUG("send_list = " << send_list_.size());

            send_list_.pop_front();
            if (!send_list_.empty())
            {
                boost::asio::async_write(socket_, boost::asio::buffer(send_list_.front().Data(), send_list_.front().Length()),
                    boost::asio::transfer_at_least(send_list_.front().Length()), boost::bind(&HttpServer::HandleTcpSend,
                    shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
            }

            if (send_list_.empty() && will_close_ == true)
            {
                HTTPSVR_DEBUG("send_set_.size() == 0 && will_close_ == true");
                DelayClose();
            }
        }
        else
        {
            boost::system::error_code err_code;
            HTTPSVR_DEBUG(" endpoint: " << socket_.remote_endpoint(err_code) << ", bytes_transferred: " << bytes_transferred << ", err: " << err.message());

            if (handler_)
            {
                handler_->OnTcpSendFailed();
            }

            Close();
        }
    }

    void HttpServer::WillClose()
    {
        if (false == is_open_)
        {
            HTTPSVR_DEBUG("is_open = false, send_list.size = " << send_list_.size());
            return;
        }

        will_close_ = true;
        HTTPSVR_DEBUG("send_list = " << send_list_.size());
        if (send_list_.empty() && send_list_.empty())
        {
            DelayClose();
        }
    }

    void HttpServer::Close()
    {
        if (is_open_ == false)
        {
            HTTPSVR_DEBUG("is_open = false");
            return;
        }

        HTTPSVR_DEBUG("sended_length = " << sended_bytes_);

        // clear
        send_list_.clear();

        recv_timer_.stop();
        close_timer_.stop();

        if (handler_)
        {
            HTTPSVR_DEBUG("Handler " << handler_);
            handler_->OnClose();
            handler_.reset();
        }

        boost::system::error_code error;
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
        if (error)
        {
            HTTPSVR_DEBUG("socket_.shutdown error = " << error.message());
        }

        //
        is_open_ = false;

        // close
        socket_.close(error);
        if (error)
        {
            HTTPSVR_DEBUG("socket_.close error = " << error.message());
        }
    }

    void HttpServer::DelayClose()
    {
        if (is_open_ == false)
        {
            HTTPSVR_DEBUG("is_open = false");
            return;
        }
        close_timer_.start();
    }

    void HttpServer::HandleCloseTimerElapsed()
    {
        if (is_open_ == false)
        {
            HTTPSVR_DEBUG("is_open = false");
            return;
        }
        Close();
    }

    boost::asio::ip::tcp::endpoint HttpServer::GetEndPoint() const
    {
        boost::asio::ip::tcp::endpoint end_point;
        boost::system::error_code ec;
        if (is_open_)
        {
            end_point = socket_.remote_endpoint(ec);
        }
        if (!ec)
            return end_point;
        else
        {
            HTTPSVR_DEBUG("GetEndPoint Error.");
            return boost::asio::ip::tcp::endpoint();
        }
    }

    void HttpServer::HttpSendHeader(uint32_t content_length, string content_type)
    {
        HTTPSVR_DEBUG("content_length: " << content_length << ", type " << content_type);

        std::ostringstream response_stream;
        response_stream << "HTTP/1.0 200 OK\r\n";
        response_stream << "Content-Type: " << content_type << "\r\n";
        response_stream << "Content-Length: " << content_length << "\r\n";
        response_stream << "Connection: close\r\n";
        response_stream << "\r\n";

        base::AppBuffer buffer(response_stream.str());
        TcpSend(buffer);
    }

    void HttpServer::HttpSendKeepAliveHeader(uint32_t content_length, string content_type)
    {
        HTTPSVR_DEBUG("content_length: " << content_length << ", type " << content_type);

        std::ostringstream response_stream;
        response_stream << "HTTP/1.0 200 OK\r\n";
        response_stream << "Content-Type: " << content_type << "\r\n";
        response_stream << "Content-Length: " << content_length << "\r\n";
        response_stream << "Connection: Keep-Alive\r\n";
        response_stream << "\r\n";

        base::AppBuffer buffer(response_stream.str());
        TcpSend(buffer);
    }

    void HttpServer::HttpSendHeader(string header_string)
    {
        base::AppBuffer buffer(header_string);
        TcpSend(buffer);
    }

    void HttpServer::HttpSendBuffer(const boost::uint8_t* data, uint32_t length)
    {
        base::AppBuffer buffer(data, length);
        TcpSend(buffer);
    }

    void HttpServer::HttpSendBuffer(const base::AppBuffer & buffer)
    {
        TcpSend(buffer);
    }

    void HttpServer::HttpSendBuffer(const protocol::SubPieceBuffer& buffer)
    {
        base::AppBuffer app_buf(buffer);
        TcpSend(app_buf);
    }

    void HttpServer::HttpSendContent(const boost::uint8_t* data, uint32_t length, string content_type)
    {
        HttpSendHeader(length, content_type);
        HttpSendBuffer(data, length);
        WillClose();
    }

    void HttpServer::HttpSendContent(const string& text, string content_type)
    {
        HttpSendContent((const boost::uint8_t*) text.c_str(), text.length(), content_type);
    }

    void HttpServer::HttpSend403Header()
    {
        HTTPSVR_DEBUG("send 403");

        std::ostringstream response_stream;
        response_stream << "HTTP 403 Forbidden\r\n";
        response_stream << "Connection: Close\r\n";
        response_stream << "\r\n";

        base::AppBuffer buffer(response_stream.str());
        TcpSend(buffer);
    }
}

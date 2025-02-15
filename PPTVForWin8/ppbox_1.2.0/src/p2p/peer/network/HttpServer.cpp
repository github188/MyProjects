//------------------------------------------------------------------------------------------
//     Copyright (c)2005-2010 PPLive Corporation.  All rights reserved.
//------------------------------------------------------------------------------------------

#include "Common.h"
#include "network/HttpServer.h"

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/placeholders.hpp>

namespace network
{
#ifdef LOG_ENABLE
    static log4cplus::Logger logger_httpserver = log4cplus::Logger::getInstance("[http_server]");
#endif

    HttpServer::HttpServer(
        boost::asio::io_service & io_svc)
        : socket_(io_svc)
        , is_open_(false)
        , will_close_(false)
    {
    }

    void HttpServer::HttpRecv()
    {
        if (is_open_ == false)
            return;

        string delim("\r\n\r\n");
        boost::asio::async_read_until(socket_, request_, delim, boost::bind(&HttpServer::HandleHttpRecv,
            shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }

    void HttpServer::HandleHttpRecv(const boost::system::error_code& err, boost::uint32_t bytes_transferred)
    {
        if (is_open_ == false)
        {
            LOG4CPLUS_DEBUG_LOG(logger_httpserver, "is_open_ = false, error = " << err.message());
            return;
        }

        if (!err)
        {
            string request_string;
            std::copy(std::istreambuf_iterator<char> (&request_), std::istreambuf_iterator<char> (), std::back_inserter(request_string));

            LOG4CPLUS_DEBUG_LOG(logger_httpserver, "RequestString:\n" << request_string << ", Handler = " << handler_);

            HttpRequest::p http_request = HttpRequest::ParseFromBuffer(request_string);
            if (!http_request)
            {
                LOG4CPLUS_DEBUG_LOG(logger_httpserver, "HttpRequest Parse Error!");
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

    void HttpServer::TcpSend(const base::AppBuffer& buffer)
    {
        if (is_open_ == false)
        {
            LOG4CPLUS_DEBUG_LOG(logger_httpserver, "send_buffer, is_open = false");
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

    void HttpServer::HandleTcpSend(const boost::system::error_code& err, boost::uint32_t bytes_transferred)
    {
        if (is_open_ == false)
        {
            return;
        }

        if (!err)
        {
            LOG4CPLUS_DEBUG_LOG(logger_httpserver, "send_list = " << send_list_.size());

            send_list_.pop_front();
            if (!send_list_.empty())
            {
                boost::asio::async_write(socket_, boost::asio::buffer(send_list_.front().Data(), send_list_.front().Length()),
                    boost::asio::transfer_at_least(send_list_.front().Length()), boost::bind(&HttpServer::HandleTcpSend,
                    shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
            }

            if (send_list_.empty() && will_close_ == true)
            {
                LOG4CPLUS_DEBUG_LOG(logger_httpserver, "send_set_.size() == 0 && will_close_ == true");
                Close();
            }
        }
        else
        {
            boost::system::error_code err_code;
            LOG4CPLUS_DEBUG_LOG(logger_httpserver, " endpoint: " << socket_.remote_endpoint(err_code) << 
                ", bytes_transferred: " << bytes_transferred << ", err: " << err.message());

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
            LOG4CPLUS_DEBUG_LOG(logger_httpserver, "is_open = false, send_list.size = " << send_list_.size());
            return;
        }

        will_close_ = true;
        LOG4CPLUS_DEBUG_LOG(logger_httpserver, "send_list = " << send_list_.size());
        if (send_list_.empty())
        {
            Close();
        }
    }

    void HttpServer::Close()
    {
        if (is_open_ == false)
        {
            LOG4CPLUS_DEBUG_LOG(logger_httpserver, "is_open = false");
            return;
        }

        // clear
        send_list_.clear();

        if (handler_)
        {
            LOG4CPLUS_DEBUG_LOG(logger_httpserver, "Handler " << handler_);
            handler_->OnClose();
            handler_.reset();
        }

        boost::system::error_code error;
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
        if (error)
        {
            LOG4CPLUS_DEBUG_LOG(logger_httpserver, "socket_.shutdown error = " << error.message());
        }

        //
        is_open_ = false;

        // close
        socket_.close(error);
        if (error)
        {
            LOG4CPLUS_DEBUG_LOG(logger_httpserver, "socket_.close error = " << error.message());
        }
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
            LOG4CPLUS_DEBUG_LOG(logger_httpserver, "GetEndPoint Error.");
            return boost::asio::ip::tcp::endpoint();
        }
    }

    void HttpServer::HttpSendHeader(boost::uint32_t content_length, string content_type)
    {
        LOG4CPLUS_DEBUG_LOG(logger_httpserver, "content_length: " << content_length << ", type " << content_type);

        std::ostringstream response_stream;
        response_stream << "HTTP/1.0 200 OK\r\n";
        response_stream << "Content-Type: " << content_type << "\r\n";
        response_stream << "Content-Length: " << content_length << "\r\n";
        response_stream << "Connection: close\r\n";
        response_stream << "\r\n";

        base::AppBuffer buffer(response_stream.str());
        TcpSend(buffer);
    }

    void HttpServer::HttpSendHeader(string header_string)
    {
        base::AppBuffer buffer(header_string);
        TcpSend(buffer);
    }

    void HttpServer::HttpSendBuffer(const boost::uint8_t* data, boost::uint32_t length)
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

    void HttpServer::HttpSendContent(const boost::uint8_t* data, boost::uint32_t length, string content_type)
    {
        HttpSendHeader(length, content_type);
        HttpSendBuffer(data, length);
        WillClose();
    }

    void HttpServer::HttpSendContent(const string& text, string content_type)
    {
        HttpSendContent((const boost::uint8_t*) text.c_str(), text.length(), content_type);
    }
}

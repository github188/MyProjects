// HttpRequest.h

#ifndef _UTIL_PROTOCOL_HTTP_HTTP_REQUEST_H_
#define _UTIL_PROTOCOL_HTTP_HTTP_REQUEST_H_

#include "util/protocol/http/HttpPacket.h"

namespace util
{
    namespace protocol
    {

        class HttpRequestHead
            : public HttpHead
        {
        public:
            enum MethodEnum
            {
                get, 
                post, 
                put, 
                head, 
                invalid_method, 
            };

        public:
            HttpRequestHead(
                MethodEnum method = get);

            HttpRequestHead(
                MethodEnum method, 
                std::string const & path);

            HttpRequestHead(
                MethodEnum method, 
                std::string const & path, 
                size_t version);

        public:
            static std::string const method_str[];

        public:
            MethodEnum method;
            std::string path;

        private:
            virtual bool get_line(
                std::string & line) const;

            virtual bool set_line(
                std::string const & line);
        };

        class HttpRequest
            : public HttpPacket
        {
        public:
            HttpRequest()
                : HttpPacket(head_)
            {
            }

            HttpRequest(
                HttpRequestHead const & head)
                : HttpPacket(head_)
                , head_(head)
            {
            }

            HttpRequest(
                HttpRequest const & r)
                : HttpPacket(head_)
                , head_(r.head())
            {
                HttpPacket::operator =(r);
            }

        public:
            HttpRequestHead & head()
            {
                return head_;
            }

            HttpRequestHead const & head() const
            {
                return head_;
            }

        private:
            HttpRequestHead head_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_HTTP_HTTP_REQUEST_H_

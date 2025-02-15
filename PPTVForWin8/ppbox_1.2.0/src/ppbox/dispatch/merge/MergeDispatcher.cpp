// MergeDispatcher.cpp

#include "ppbox/dispatch/Common.h"
#include "ppbox/dispatch/merge/MergeDispatcher.h"
#include "ppbox/dispatch/merge/MergeTask.h"
#include "ppbox/dispatch/Error.h"

#include <ppbox/merge/MergeModule.h>
#include <ppbox/merge/MergeError.h>

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>

#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("ppbox.dispatch.MergeDispatcher", framework::logger::Debug);

namespace ppbox
{
    namespace dispatch
    {

        MergeDispatcher::MergeDispatcher(
            boost::asio::io_service & io_svc, 
            boost::asio::io_service & dispatch_io_svc)
            : TaskDispatcher(io_svc, dispatch_io_svc)
            , merge_module_(util::daemon::use_module<ppbox::merge::MergeModule>(io_svc))
            , merge_close_token_(0)
            , merger_(NULL)
            , cancel_token_(false)
            , pause_token_(false)
        {
        }

        MergeDispatcher::~MergeDispatcher()
        {
        }

        void MergeDispatcher::start_open(
            framework::string::Url const & url)
        {
            LOG_DEBUG("[start_open] playlink:"<< url.param(param_playlink));
            merge_module_.async_open(
                framework::string::Url(url.param(param_playlink)), 
                merge_close_token_, 
                boost::bind(&MergeDispatcher::handle_open, this, url, _1, _2));
        }

        void MergeDispatcher::handle_open(
            framework::string::Url const & url, 
            boost::system::error_code const & ec, 
            ppbox::merge::MergerBase * merger)
        {
            LOG_DEBUG("[handle_open] ec:" << ec.message());
            merger_ = merger;
            boost::system::error_code ec1 = ec;
            if (!ec1) {
                format_ = url.param(param_format);
                framework::string::Url::param_const_iterator iter = url.param_begin();
                for (; iter != url.param_end(); ++iter) {
                    if (iter->key().compare(0, 6, "merge.") == 0) {
                        std::string::size_type pos_dot = iter->key().rfind('.');
                        if (pos_dot == 5)
                            continue;
                        merger_->config().set(
                            iter->key().substr(6, pos_dot - 6), 
                            iter->key().substr(pos_dot + 1), 
                            iter->value());
                    }
                }
            }
            response(ec1);
        }

        void MergeDispatcher::cancel_open(
            boost::system::error_code & ec)
        {
            LOG_DEBUG("[cancel_open]");
            merge_module_.close(merge_close_token_,ec);
            merge_close_token_ = 0;
            merger_ = NULL;
        }

        void MergeDispatcher::do_setup(
            boost::uint32_t index, 
            boost::system::error_code & ec)
        {
            LOG_DEBUG("[do_setup]");
            //muxer_->setup(index, ec);
            assert(index == (boost::uint32_t)-1);
            ec.clear();
        }

        void MergeDispatcher::start_play(
            SeekRange const & range, 
            response_t const & seek_resp)
        {
            LOG_DEBUG("[start_play]");
            dispatch_io_svc().post(MergeTask(task_info(), sink_group(), range, seek_resp, 
                boost::bind(&MergeDispatcher::handle_play, this, _1), merger_));
        }

        void MergeDispatcher::handle_play(
            boost::system::error_code const & ec)
        {
            LOG_DEBUG("[handle_play] ec:" << ec.message());
            boost::system::error_code ec1 = ec;
            if (ec1 == ppbox::merge::error::end_of_file) {
                ec1.clear();
            }
            response(ec1);
        }

        void MergeDispatcher::start_buffer()
        {
            LOG_DEBUG("[start_buffer]");
            dispatch_io_svc().post(MergeTask(task_info(), 
                boost::bind(&MergeDispatcher::handle_buffer, this, _1), merger_));
        }

        void MergeDispatcher::handle_buffer(
            boost::system::error_code const & ec)
        {
            LOG_DEBUG("[handle_buffer] ec:"<<ec.message());
            response(ec);
        }

        void MergeDispatcher::do_close(
            boost::system::error_code & ec)
        {
            LOG_DEBUG("[do_close]");
            if (merge_close_token_) {
                merge_module_.close(merge_close_token_, ec);
                merge_close_token_ = 0;
                merger_ = NULL;
            }
        }

        bool MergeDispatcher::get_media_info(
            MediaInfo & info, 
            boost::system::error_code & ec)
        {
            LOG_DEBUG("[get_media_info]");
            merger_->media_info(info);
            ec.clear();
            return true;
        }

        bool MergeDispatcher::get_stream_info(
            std::vector<StreamInfo> & streams, 
            boost::system::error_code & ec)
        {
            LOG_DEBUG("[get_stream_info]");
            merger_->stream_info(streams);
            ec.clear();
            return true;
        }

        void MergeDispatcher::do_get_stream_status(
            StreamStatus & info, 
            boost::system::error_code & ec)
        {
            LOG_DEBUG("[do_get_stream_status]");
            merger_->stream_status(info);
            ec.clear();
        }

        bool MergeDispatcher::accept(
            framework::string::Url const & url)
        {
            // TO BE FIX
            return url.param(param_format) == "mp4";
        }

        bool MergeDispatcher::assign(
            framework::string::Url const & url, 
            boost::system::error_code & ec)
        {
            TaskDispatcher::assign(url, ec);
            std::string format = url.param(param_format);
            if (format_ != format) {
                ec = ppbox::merge::error::format_not_match;
            }
            if (!ec) {
                merger_->reset(ec);
                if (ec == boost::asio::error::would_block)
                    ec.clear();
            }
            return !ec;
        }

    } // namespace mux
} // namespace ppbox

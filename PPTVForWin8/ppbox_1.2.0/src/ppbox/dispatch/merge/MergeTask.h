// MergeTask.h

#ifndef _PPBOX_DISPATCH_MERGE_MERGE_TASK_H_
#define _PPBOX_DISPATCH_MERGE_MERGE_TASK_H_

#include "ppbox/dispatch/Task.h"

#include <ppbox/merge/MergerBase.h>

namespace ppbox
{

    namespace dispatch
    {

        class MergeTask 
            : public Task<MergeTask>
        {
        public:
            MergeTask(
                TaskInfo & info, 
                SinkGroup & sinks, 
                SeekRange const & range, 
                response_t const & seek_resp, 
                response_t const & resp, 
                ppbox::merge::MergerBase * merger)
                : Task<MergeTask>(info, sinks, range, seek_resp, resp)
                , merger_(merger)
            {
            }
            
            MergeTask(
                TaskInfo & info, 
                response_t const & resp, 
                ppbox::merge::MergerBase * merger)
                : Task<MergeTask>(info, resp)
                , merger_(merger)
            {
            }

            bool read_sample(
                ppbox::avformat::Sample & sample, 
                boost::system::error_code & ec)
            {
                merger_->read(sample, ec);
                return !ec;
            }

            bool buffer(
                boost::system::error_code & ec)
            {
                boost::system::error_code ec1;
                merger_->get_buffer_time(ec1, ec);
                return !ec;
            }

            bool byte_seek(
                boost::uint64_t beg, 
                boost::uint64_t end, 
                boost::system::error_code & ec)
            {
                return merger_->byte_seek(beg, ec);
            }
/*
            bool time_seek(
                boost::uint64_t beg, 
                boost::uint64_t end, 
                boost::system::error_code & ec)
            {
                return merger_->time_seek(beg, ec);
            }
*/
            boost::uint64_t check_seek(
                boost::system::error_code & ec)
            {
                boost::system::error_code ec_buf;
                merger_->get_buffer_time(ec, ec_buf);
                return 0;
            }

        private:
            ppbox::merge::MergerBase * merger_;
        };

    } // namespace dispatch
} // namespace ppbox

#endif // _PPBOX_DISPATCH_MERGE_MERGE_TASK_H_

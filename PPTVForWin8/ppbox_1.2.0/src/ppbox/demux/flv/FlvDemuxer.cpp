// FlvDemuxer.cpp

#include "ppbox/demux/Common.h"
#include "ppbox/demux/flv/FlvDemuxer.h"
using namespace ppbox::demux::error;
using namespace ppbox::avformat;

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>
#include <framework/system/BytesOrder.h>
#include <framework/timer/TimeCounter.h>
using namespace framework::system;
using namespace boost::system;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("ppbox.demux.FlvDemuxer", framework::logger::Warn)

#include <iostream>

namespace ppbox
{
    namespace demux
    {

        FlvDemuxer::FlvDemuxer(
            boost::asio::io_service & io_svc, 
            std::basic_streambuf<boost::uint8_t> & buf)
            : Demuxer(io_svc, buf)
            , archive_(buf)
            , open_step_((size_t)-1)
            , header_offset_(0)
            , parse_offset_(0)
            , timestamp_offset_ms_(0)
            , current_time_(0)
        {
            streams_.resize(2);
        }

        FlvDemuxer::~FlvDemuxer()
        {
        }

        error_code FlvDemuxer::open(
            error_code & ec)
        {
            open_step_ = 0;
            parse_offset_ = 0;
            ec.clear();
            is_open(ec);
            return ec;
        }

        bool FlvDemuxer::is_open(
            error_code & ec)
        {
            if (open_step_ == 3) {
                ec = error_code();
                return true;
            }

            if (open_step_ == (size_t)-1) {
                ec = error::not_open;
                return false;
            }

            assert(archive_);

            if (open_step_ == 0) {
                archive_.seekg(0, std::ios_base::beg);
                assert(archive_);
                archive_ >> flv_header_;
                if (archive_) {
                    streams_.resize((size_t)FlvTagType::DATA + 1);
                    if (flv_header_.TypeFlagsAudio) {
                        streams_[(size_t)FlvTagType::AUDIO].index = stream_map_.size();
                        stream_map_.push_back((size_t)FlvTagType::AUDIO);
                    }
                    if (flv_header_.TypeFlagsVideo) {
                        streams_[(size_t)FlvTagType::VIDEO].index = stream_map_.size();
                        stream_map_.push_back((size_t)FlvTagType::VIDEO);
                    }
                    open_step_ = 1;
                    parse_offset_ = std::ios::off_type(flv_header_.DataOffset) + 4; // + 4 PreTagSize
                } else {
                    if (archive_.failed()) {
                        ec = error::bad_file_format;
                    } else {
                        ec = error::file_stream_error;
                    }
                    archive_.clear();
                }
            }

            if (open_step_ == 1) {
                archive_.seekg(parse_offset_, std::ios_base::beg);
                assert(archive_);
                while (!get_tag(flv_tag_, ec)) {
                    if (flv_tag_.Type == FlvTagType::DATA 
                        && flv_tag_.DataTag.Name.String == "onMetaData") {
                        metadata_.from_data(flv_tag_.DataTag.Value);
                    }
                    std::vector<boost::uint8_t> codec_data;
                    archive_.seekg(std::streamoff(flv_tag_.data_offset), std::ios_base::beg);
                    assert(archive_);
                    util::serialization::serialize_collection(archive_, codec_data, (boost::uint64_t)flv_tag_.DataSize);
                    archive_.seekg(4, std::ios_base::cur);
                    assert(archive_);
                    parse_offset_ = (boost::uint64_t)archive_.tellg();
                    boost::uint32_t index = streams_[(size_t)flv_tag_.Type].index;
                    streams_[(size_t)flv_tag_.Type] = FlvStream(flv_tag_, codec_data, metadata_);
                    streams_[(size_t)flv_tag_.Type].index = index;
                    streams_[(size_t)flv_tag_.Type].ready = true;
                    bool ready = true;
                    for (size_t i = 0; i < stream_map_.size(); ++i) {
                        if (!streams_[stream_map_[i]].ready) {
                            ready = false;
                            break;
                        }
                    }
                    if (ready)
                        break;
                }
                if (!ec) {
                    if (timestamp_offset_ms_ == 0) {
                        open_step_ = 2;
                    } else {
                        header_offset_ = parse_offset_;
                        open_step_ = 3;
                    }
                }
            }

            if (open_step_ == 2) {
                archive_.seekg(parse_offset_, std::ios_base::beg);
                assert(archive_);
                while (!get_tag(flv_tag_, ec)) {
                    if (flv_tag_.Type < streams_.size() &&
                        streams_[(size_t)flv_tag_.Type].index < stream_map_.size()) {
                            break;
                    }
                }
                if (!ec) {
                    archive_.seekg(parse_offset_, std::ios_base::beg);
                    assert(archive_);
                    current_time_ = timestamp_offset_ms_ = flv_tag_.Timestamp;
                    for (size_t i = 0; i < stream_map_.size(); ++i) {
                        streams_[stream_map_[i]].start_time = timestamp_offset_ms_;
                    }
                    header_offset_ = parse_offset_;
                    open_step_ = 3;
                }
            }

            if (ec) {
                archive_.clear();
                archive_.seekg(parse_offset_, std::ios_base::beg);
                assert(archive_);
                return false;
            } else {
                if (3 == open_step_) {
                    on_open();
                    boost::uint64_t duration = 0;
                    boost::uint64_t filesize = 0;
                    if (metadata_.duration != 0) {
                        duration = metadata_.duration * 1000; // ms
                    }
                    if (metadata_.filesize != 0) {
                        filesize = metadata_.filesize;
                    }
                }
            }

            return true;
        }

        bool FlvDemuxer::is_open(
            error_code & ec) const
        {
            if (open_step_ == 3) {
                ec = error_code();
                return true;
            } else {
                ec = error::not_open;
                return false;
            }
        }

        error_code FlvDemuxer::close(error_code & ec)
        {
            streams_.clear();
            stream_map_.clear();
            header_offset_ = 0;
            current_time_ = timestamp_offset_ms_ = 0;
            parse_offset_ = 0;
            ec.clear();
            open_step_ = size_t(-1);
            return ec;
        }

        error_code FlvDemuxer::reset(
            error_code & ec)
        {
            ec.clear();
            parse_offset_ = header_offset_;
            return ec;
        }

        boost::uint64_t FlvDemuxer::seek(
            boost::uint64_t & time, 
            boost::uint64_t & delta, 
            error_code & ec)
        {
            if (is_open(ec)) {
                //if (time == 0) {
                    ec.clear();
                    time = 0;
                    current_time_ = timestamp_offset_ms_;
                    parse_offset_ = header_offset_;
                    return header_offset_;
                //} else {
                //    ec = error::not_support;
                //    return 0;
                //}
            } else {
                return 0;
            }
        }

        boost::uint64_t FlvDemuxer::get_duration(
            error_code & ec) const
        {
            ec = error::not_support;
            return ppbox::data::invalid_size;
        }

        size_t FlvDemuxer::get_stream_count(
            error_code & ec) const
        {
            if (is_open(ec)) {
                return stream_map_.size();
            } else {
                return 0;
            }
        }

        error_code FlvDemuxer::get_stream_info(
            size_t index, 
            StreamInfo & info, 
            error_code & ec) const
        {
            if (is_open(ec)) {
                if (index >= stream_map_.size()) {
                    ec = framework::system::logic_error::out_of_range;
                } else {
                    info = streams_[stream_map_[index]];
                }
            }
            return ec;
        }

        error_code FlvDemuxer::get_sample(
            Sample & sample, 
            error_code & ec)
        {
            if (!is_open(ec)) {
                return ec;
            }
            archive_.seekg(parse_offset_, std::ios_base::beg);
            assert(archive_);
            framework::timer::TimeCounter tc;
            if (get_tag(flv_tag_, ec)) {
                archive_.seekg(parse_offset_, std::ios_base::beg);
                assert(archive_);
                return ec;
            }
            if (flv_tag_.Type == FlvTagType::VIDEO) {
                current_time_ = flv_tag_.Timestamp;
            }
            if (tc.elapse() > 10) {
                LOG_DEBUG("[get_tag], elapse " << tc.elapse());
            }
            parse_offset_ = (boost::uint64_t)archive_.tellg();
            if (flv_tag_.is_sample) {
                assert(flv_tag_.Type < streams_.size() && 
                    streams_[(size_t)flv_tag_.Type].index < stream_map_.size());
                FlvStream const & stream = streams_[(size_t)flv_tag_.Type];
                sample.itrack = stream.index;
                sample.flags = 0;
                if (flv_tag_.is_sync)
                    sample.flags |= Sample::sync;
                sample.dts = timestamp_.transfer((boost::uint64_t)flv_tag_.Timestamp);;
                sample.cts_delta = flv_tag_.cts_delta;
                sample.duration = 0;
                Demuxer::adjust_timestamp(sample);
                sample.size = flv_tag_.DataSize;
                sample.blocks.clear();
                sample.blocks.push_back(FileBlock(flv_tag_.data_offset, flv_tag_.DataSize));
            } else if (flv_tag_.Type == FlvTagType::DATA) {
                LOG_DEBUG("[get_sample] script data: " << flv_tag_.DataTag.Name.String.StringData);
                return get_sample(sample, ec);
            } else if (flv_tag_.Type == FlvTagType::AUDIO) {
                if (flv_tag_.AudioHeader.SoundFormat == FlvSoundCodec::AAC
                    && flv_tag_.AudioHeader.AACPacketType == 0) {
                        LOG_DEBUG("[get_sample] duplicate aac sequence header");
                        return get_sample(sample, ec);
                }
                ec = bad_file_format;
            } else if (flv_tag_.Type == FlvTagType::VIDEO) {
                if (flv_tag_.VideoHeader.CodecID == FlvVideoCodec::H264) {
                    if (flv_tag_.VideoHeader.AVCPacketType == 0) {
                        LOG_DEBUG("[get_sample] duplicate aac sequence header");
                        return get_sample(sample, ec);
                    } else if (flv_tag_.VideoHeader.AVCPacketType == 2) {
                        LOG_DEBUG("[get_sample] end of avc sequence");
                        return get_sample(sample, ec);
                    }
                } 
                ec = bad_file_format;
            } else {
                ec = bad_file_format;
            }
            return ec;
        }

        boost::uint64_t FlvDemuxer::get_cur_time(
            error_code & ec) const
        {
            if (is_open(ec)) {
                // return flv_tag_.Timestamp - timestamp_offset_ms_;
                 return current_time_ - timestamp_offset_ms_;
            }
            return 0;
        }

        boost::uint64_t FlvDemuxer::get_end_time(
            error_code & ec)
        {
            if (!is_open(ec)) {
                return 0;
            }
            boost::uint64_t beg = archive_.tellg();
            archive_.seekg(0, std::ios_base::end);
            assert(archive_);
            boost::uint64_t end = archive_.tellg();
            archive_.seekg(beg, std::ios_base::beg);
            assert(archive_);
            boost::uint64_t time = 0;
            if (flv_tag_.Timestamp - timestamp_offset_ms_ > 1000) {
                time = (flv_tag_.Timestamp - timestamp_offset_ms_) * end / parse_offset_;
            } else if (metadata_.datarate) {
                time = end * 8 / metadata_.datarate;
            } else {
                time = 0;
            }
            return time;
        }

        error_code FlvDemuxer::get_tag(
            FlvTag & flv_tag,
            error_code & ec)
        {
            if (archive_ >> flv_tag) {
                ec.clear();
                return ec;
            } else if (archive_.failed()) {
                archive_.clear();
                return ec = error::bad_file_format;
            } else {
                archive_.clear();
                return ec = error::file_stream_error;
            }
        }

    }
}

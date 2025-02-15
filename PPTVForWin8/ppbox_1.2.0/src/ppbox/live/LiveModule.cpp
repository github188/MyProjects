// LiveModule.cpp

#include "ppbox/live/Common.h"
#include "ppbox/live/LiveModule.h"
#ifndef PPBOX_DISABLE_DAC
#include <ppbox/dac/DacModule.h>
#include <ppbox/dac/DacInfoWorker.h>
using namespace ppbox::dac;
#endif
//#include <ppbox/live_worker/Version.h>
#include <ppbox/live_worker/Name.h>


#ifdef PPBOX_CONTAIN_LIVE_WORKER
#include <ppbox/live_worker/LiveProxy.h>
using namespace ppbox::live_worker;
#else
#include <framework/process/Process.h>
#include <framework/timer/Timer.h>
using namespace framework::process;
using namespace framework::timer;
#endif

#include <framework/system/ErrorCode.h>
#include <framework/system/LogicError.h>
#include <framework/string/Format.h>
#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>
using namespace framework::system;
using namespace framework::string;

#include <boost/bind.hpp>
using namespace boost::system;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("ppbox.live.LiveModule", framework::logger::Debug)

namespace ppbox
{
    namespace live
    {

        LiveModule::LiveModule(

            util::daemon::Daemon & daemon)
            : ppbox::common::CommonModuleBase<LiveModule>(daemon, "LiveModule")
#ifndef PPBOX_DISABLE_DAC
            , dac_(util::daemon::use_module<ppbox::dac::DacModule>(daemon))
#endif
            , portMgr_(util::daemon::use_module<ppbox::common::PortManager>(daemon))
            , port_(9001)
#ifndef PPBOX_CONTAIN_LIVE_WORKER
            , mutex_(9001)
            , is_locked_(false)
#endif
        {

#ifdef PPBOX_CONTAIN_LIVE_WORKER
            util::daemon::use_module<ppbox::live_worker::LiveProxy>(daemon);
#else
            process_ = new Process;
            timer_ = new Timer(timer_queue(), 
                10, // 5 seconds
                boost::bind(&LiveModule::check, this));
#endif
        }

        LiveModule::~LiveModule()
        {
#ifndef PPBOX_CONTAIN_LIVE_WORKER
            if (is_lock()) {
                mutex_.unlock();
                is_locked_ = false;
            }
            if (process_) {
                delete process_;
                process_ = NULL;
            }
            if (timer_) {
                delete timer_;
                timer_ = NULL;
            }
#endif
        }

        error_code LiveModule::startup()
        {
            error_code ec;
            LOG_INFO("[startup]");
#ifndef PPBOX_DISABLE_DAC
            dac_.set_live_version(version());
            dac_.set_live_name(name());
#endif
#ifndef PPBOX_CONTAIN_LIVE_WORKER
            timer_->start();

            if (is_lock()) {
                LOG_INFO("[startup] try_lock");

                boost::filesystem::path cmd_file(ppbox::live_worker::name_string());
                Process::CreateParamter param;
                param.wait = true;
                process_->open(cmd_file, param, ec);
                if (!ec) {
                    portMgr_.get_port(ppbox::common::live,port_);
                    LOG_INFO("[startup] ok port:"<<port_);
                } else {
                    LOG_WARN("[startup] ec = " << ec.message());
                    //port_ = 0;
                    if (ec == boost::system::errc::no_such_file_or_directory) {
                        ec.clear();
                    }

                    timer_->stop();
                }
            }
#endif
            return ec;
        }

        void LiveModule::check()
        {
#ifndef PPBOX_CONTAIN_LIVE_WORKER
            error_code ec;
            if (is_lock()) {
                if (process_ && !process_->is_alive(ec)) {
                    LOG_ERROR("[check] worker is dead: " << ec.message());

#ifndef PPBOX_DISABLE_DAC
                    util::daemon::use_module<ppbox::dac::DacModule>(get_daemon())
                        .submit(DacRestartInfo(DacRestartInfo::live));
#endif
                    process_->close(ec);
                    boost::filesystem::path cmd_file(ppbox::live_worker::name_string());
                    Process::CreateParamter param;
                    param.wait = true;
                    process_->open(cmd_file, param, ec);
                    if (!ec) {
                        portMgr_.get_port(ppbox::common::live,port_);
                        LOG_INFO("[check] ok port:"<<port_);
                    } else {
                        LOG_WARN("[check] ec = " << ec.message());
                        port_ = 0;

                        timer_->stop();
                    }
                }
            }
#else
          portMgr_.get_port(ppbox::common::live,port_);
          LOG_INFO("[check] ok port:"<<port_);	
#endif
        }

        bool LiveModule::is_alive()
        {
            error_code ec;
#ifdef PPBOX_CONTAIN_LIVE_WORKER
            return true;
#else
            if (is_locked_) {
                return process_ && process_->is_alive(ec);
            } else {
                framework::process::Process process;
                boost::filesystem::path cmd_file(ppbox::live_worker::name_string());
                process.open(cmd_file, ec);
                return !ec;
            }
#endif
        }

        void LiveModule::shutdown()
        {
#ifndef PPBOX_CONTAIN_LIVE_WORKER
            error_code ec;
            if (process_) {
                error_code ec;
                process_->signal(Signal::sig_int, ec);
                process_->timed_join(1000, ec);
                if (!ec) {
                    LOG_INFO("[shutdown] ok");
                } else {
                    LOG_WARN("[shutdown] ec = " << ec.message());
                }
                process_->kill(ec);
                process_->close(ec);
            }
            if (timer_) {
                timer_->stop();
            }
            if (is_locked_) {
                mutex_.unlock();
                is_locked_ = false;
            }
#endif
        }

        std::string LiveModule::version()
        {
            //return ppbox::live_worker::version_string();
            return "1.0.0.1";
        }

        std::string LiveModule::name()
        {
            return ppbox::live_worker::name_string();
        }

#ifndef PPBOX_CONTAIN_LIVE_WORKER
        bool LiveModule::is_lock()
        {
            if (!is_locked_) {
                is_locked_ = mutex_.try_lock();
            }

            return is_locked_;
        }
#endif

    } // namespace live
} // namespace ppbox

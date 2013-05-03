//------------------------------------------------------------------------------------------
//     Copyright (c)2005-2010 PPLive Corporation.  All rights reserved.
//------------------------------------------------------------------------------------------

#ifndef ASIO_SERVICE_RUNNER_H
#define ASIO_SERVICE_RUNNER_H


class AsioServiceRunner
    : public boost::enable_shared_from_this<AsioServiceRunner>
{
public:
    AsioServiceRunner(const std::string& service_name);
    
    boost::shared_ptr<boost::asio::io_service> Start();
    void Stop();

private:
    void ServiceThreadProc();

private:
    boost::shared_ptr<boost::asio::io_service> io_service_;
    boost::shared_ptr<boost::thread> thread_;
    const std::string service_name_;
};


#endif //SUPER_NODE_ASIO_SERVICE_RUNNER_H
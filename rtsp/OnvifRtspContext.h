#ifndef __ONVIF_RTSP_CONTEXT_H__
#define __ONVIF_RTSP_CONTEXT_H__

#include <swift/utils/context.hpp>
// 完成后删除这俩头文件
#include <swift/net/rtsp/rtsp.hpp>
using namespace swift::net::rtsp;
#include <swift/net/rtp/rtp.hpp>
using namespace swift::net::rtp;

class OnvifRtspClient;

class AsyncRequestContext : public swift::context
{
public:
    AsyncRequestContext(OnvifRtspClient &client);

    virtual void OnResponse(response_ptr resp, request_ptr req);
    virtual void OnResponse(response_ptr resp, request_ptr req, client_session_ptr session);
    virtual void OnResponse(client_session_ptr session);

    // context interface
protected:
    void OnHolderDestroy();

protected:
    OnvifRtspClient &m_getReplyClient;
};

class OptionContext : public AsyncRequestContext
{
public:
    OptionContext(OnvifRtspClient &client);

    virtual void OnResponse(response_ptr resp, request_ptr req, client_session_ptr session);
};

class DescribeContext : public AsyncRequestContext
{
public:
    DescribeContext(OnvifRtspClient &client);

    void sendSetupRequests(const std::string &sdp);
    virtual void OnResponse(response_ptr resp, request_ptr req);
};

class SetupContext : public AsyncRequestContext
{
public:
    SetupContext(OnvifRtspClient &client);

    virtual void OnResponse(response_ptr resp, request_ptr req, client_session_ptr session);
};

class PlayContext : public AsyncRequestContext
{
public:
    PlayContext(OnvifRtspClient &client);

    virtual void OnResponse(response_ptr resp, request_ptr req, client_session_ptr session);
};

class TeardownContext : public AsyncRequestContext
{
public:
    TeardownContext(OnvifRtspClient &client);

    virtual void OnResponse(response_ptr resp, request_ptr req, client_session_ptr session);
};

#endif ///__ONVIF_RTSP_CONTEXT_H__

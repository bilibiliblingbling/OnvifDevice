#ifndef __ONVIF_RTSP_CLIENT_H__
#define __ONVIF_RTSP_CLIENT_H__

#include "OnvifRtspContext.h"

#include <swift/net/rtp/rtp.hpp>
using namespace swift::net::rtp;
#include <swift/net/rtsp/rtsp.hpp>
using namespace swift::net::rtsp;
#include <swift/net/rtsp/rtsp_client.hpp>

#include "../OnvifStream.h"
#include "../OnvifDeviceHelper.h"
#include "../../../include/DeviceLayerDef.h"

typedef swift::shared_ptr<receiver> ReceiverPtr;
typedef swift::shared_ptr<client> ClientImplPtr;

class OnvifDeviceControl;

class OnvifRtspClient : public client_callback, public receiver_callback
{
public:
    OnvifRtspClient(const rtsp_url &streamURI, OnvifInputStreamPtr &pStream, SoapAuthentication auth,swift::engine &engine);
    ~OnvifRtspClient();

    // DeviceControl interface
public:
    void OpenStream(IDLPContext *pContext);
    void CloseStream(IDLPContext *pContext);

    // AsyncContext callback
public:
    void Setup(const rtsp_url &setupUri, const transport_value &trans);
    void Play();

    void OnStreamOpened();
    void OnStreamClosed();
    void OnOpenSussiced();
    void OnOpenFailed();
    void OnCloseSuccessed();
    void OnCloseFailed();

    void SendRequest(request_ptr req);
    void SendAuthenticationRequest(request_ptr req, response_ptr resp, swift::context *ctxt);
    client_session_ptr GetSession();
    void SetSession(client_session_ptr session);

    OnvifInputStreamPtr GetStream();
    void SetStream(OnvifInputStreamPtr pStream);
    swift::net::ip::endpoint getLocalEndpoint();
private:
    void SetLocalEndpoint();

    // receiver_callback interface
private:
    void on_rtp_packet(rtp_packet &packet, const swift::net::ip::endpoint &remote_endpoint, receiver &owner);

    // client_callback interface
private:
    void on_response(response_ptr resp, request_ptr req, client &owner);
    response_ptr on_request(request_ptr req, client &owner);
    void on_request_timeout(request_ptr req, unsigned int millisec, client &owner);
    void on_request_send_error(request_ptr req, const swift::error_code &ec, client &owner);
    void on_not_matched_response(response_ptr resp, client &owner);

    void on_session_setup(client_session_ptr sess, request_ptr req, response_ptr resp, client &owner);
    void on_session_response(response_ptr resp, request_ptr req, client_session_ptr sess, client &owner);
    response_ptr on_session_request(request_ptr req, client_session_ptr sess, client &owner);
    void on_session_request_timeout(request_ptr req, unsigned int millisec, client_session_ptr sess, client &owner);
    void on_session_request_send_error(request_ptr req, const swift::error_code &ec, client_session_ptr sess, client &owner);
    void on_session_teardown(client_session_ptr sess, client &owner);

private:
    swift::mutex m_streamLock;

    ClientImplPtr m_pClientImpl;
    ReceiverPtr m_pReceiver;
    // 无多会话概念, 此处仅保留当前session.
    client_session_ptr m_pSession;
    OnvifInputStreamPtr m_pStream;
    IDLPContext *m_pContext;

    SoapAuthentication m_authInfo;
    swift::net::rtsp::rtsp_url m_streamUri;
    swift::net::ip::endpoint m_localEp;
};

typedef swift::shared_ptr<OnvifRtspClient> OnvifRtspClientPtr;

#endif ///__ONVIF_RTSP_CLIENT_H__

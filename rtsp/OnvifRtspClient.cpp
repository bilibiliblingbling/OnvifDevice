#include "OnvifRtspClient.h"

#include <StreamSockPool.h>
#include <swift/net/rtsp/rtsp_core.hpp>

#include <swift/net/hlp_core/hlp_www_authentication.hpp>
#define PROFILE_SPECIFICATION "application/sdp" //header "accepts"
#define DEFAULT_RTSP_PORT 554 //RTSP default prot

using namespace swift::net::rtsp;
OnvifRtspClient::OnvifRtspClient(const rtsp_url &streamURI, OnvifInputStreamPtr &pStream, SoapAuthentication auth, swift::engine &engine)
    : m_streamUri(streamURI)
    , m_pStream(pStream)
    , m_authInfo(auth)
{
    SetLocalEndpoint();
}

OnvifRtspClient::~OnvifRtspClient()
{
}

void OnvifRtspClient::OpenStream(IDLPContext *pContext)
{
    swift::mutex::lock_guard lock(m_streamLock);
    if (m_pStream->IsOpened())
        return;

    m_pContext = pContext;

    if (!m_pClientImpl.valid()) {
        m_pClientImpl = ClientImplPtr(new client(swift::net::ip::endpoint(m_streamUri.get_host(), m_streamUri.get_port())
                                                , *this, DeviceLayer::EngineInstance::Instance().GetEngine()));
    }

    m_pClientImpl->describe(m_streamUri, PROFILE_SPECIFICATION, new DescribeContext(*this));
}

void OnvifRtspClient::CloseStream(IDLPContext *pContext)
{
    if (!m_pStream->IsOpened())
        return;

    if (m_pContext = pContext) {
        if (m_pSession)
            m_pSession->teardown();
    }
}

void OnvifRtspClient::OnStreamOpened()
{
   // swift::mutex::lock_guard lock(m_streamLock);

    if (!m_pReceiver.valid()) {
        m_pReceiver = ReceiverPtr(new receiver(m_localEp, *this, DeviceLayer::EngineInstance::Instance().GetEngine()));
    }
    m_pReceiver->start_receive();
}

void OnvifRtspClient::OnStreamClosed()
{

}

void OnvifRtspClient::OnOpenSussiced()
{
    m_pStream->DoStreamOpenedCallback(m_pContext);
}

void OnvifRtspClient::Setup(const rtsp_url &setupUri,const transport_value &trans)
{
    m_pClientImpl->setup(setupUri, trans, new SetupContext(*this));
}

void OnvifRtspClient::Play()
{
    if (m_pSession.valid()) {
        swift::net::rtsp::request_ptr playReq = m_pSession->make_play_request();
        playReq->set_context(new PlayContext(*this));
        m_pSession->send_request(playReq);
    }
}

void OnvifRtspClient::OnOpenFailed()
{
    swift::mutex::lock_guard lock(m_streamLock);
    if (m_pSession)
        m_pSession.reset();

    if (m_pClientImpl.valid())
        m_pClientImpl.reset();

    if (m_pReceiver.valid())
        m_pReceiver.reset();

    if (m_pStream) {
        m_pStream->DoStreamOpenFailedCallback(DeviceLayer::DLR_E_FAIL ,m_pContext);
    }
}

void OnvifRtspClient::OnCloseSuccessed()
{
    swift::mutex::lock_guard lock(m_streamLock);
    if (m_pStream) {
        m_pStream->DoStreamClosedCallback(m_pContext);
    }

    if (m_pClientImpl.valid())
        m_pClientImpl.reset();

    if (m_pReceiver.valid())
        m_pReceiver.reset();

    if (m_pSession)
        m_pSession.reset();
}

void OnvifRtspClient::OnCloseFailed()
{
    m_pStream->DoStreamCloseFailedCallback(DeviceLayer::DLR_E_FAIL, m_pContext);
}

void OnvifRtspClient::SendRequest(request_ptr req)
{
    m_pClientImpl->send_request(req);
}

void OnvifRtspClient::SendAuthenticationRequest(request_ptr req, response_ptr resp, swift::context *ctxt)
{
    authentication::auth_ctxt_ptr auth = authentication::make_authentication_context(resp, authentication::auth_mode_basic);
    if (auth) {
        auth->set_authentication(m_authInfo.m_username.c_str(), m_authInfo.m_password.c_str());
        auth->fill_request(req);
        req->remove_header(swift::net::rtsp::h_CSeq);
    }
    if (ctxt)
        m_pClientImpl->send_request(req);
}

client_session_ptr OnvifRtspClient::GetSession()
{
    return m_pSession;
}

void OnvifRtspClient::SetSession(client_session_ptr session)
{
    swift::mutex::lock_guard lock(m_streamLock);
    if (m_pSession)
        m_pSession.reset();
    m_pSession = session;
}

OnvifInputStreamPtr OnvifRtspClient::GetStream()
{
    return m_pStream;
}

void OnvifRtspClient::SetStream(OnvifInputStreamPtr pStream)
{
    swift::mutex::lock_guard lock(m_streamLock);
    if (m_pStream->IsOpened()) {
        m_pStream->Close(m_pContext);
    }

    m_pStream = pStream;
}

void OnvifRtspClient::SetLocalEndpoint()
{
    UdpSocketPtr pSocket = StreamSockPool::Instance().CreateUdpSocket(AUTO_SELECT_PORT, SOCKET_FOR_RECV, CREATE_SOCKET_NEW);
    m_localEp = pSocket->get_local_endpoint();
    StreamSockPool::Instance().RemoveUdpSocket(pSocket);
}

swift::net::ip::endpoint OnvifRtspClient::getLocalEndpoint()
{
    return m_localEp;
}

void OnvifRtspClient::on_rtp_packet(rtp_packet &packet, const swift::net::ip::endpoint &remote_endpoint, receiver &owner)
{
    ONVIF_DEVICE_UNUSED(remote_endpoint);
    ONVIF_DEVICE_UNUSED(owner);

    m_pStream->OnStreamData(packet.get_data().data(), packet.get_data().length());
}

void OnvifRtspClient::on_response(response_ptr resp, request_ptr req, client &owner)
{
    AsyncRequestContext *ctxt = dynamic_cast<AsyncRequestContext*>(req->get_context());
    if (ctxt)
        ctxt->OnResponse(resp, req);
}

response_ptr OnvifRtspClient::on_request(request_ptr req, client &owner)
{
}

void OnvifRtspClient::on_request_timeout(request_ptr req, unsigned int millisec, client &owner)
{
}

void OnvifRtspClient::on_request_send_error(request_ptr req, const swift::error_code &ec, client &owner)
{
}

void OnvifRtspClient::on_not_matched_response(response_ptr resp, client &owner)
{
}

void OnvifRtspClient::on_session_setup(client_session_ptr sess, request_ptr req, response_ptr resp, client &owner)
{
}

void OnvifRtspClient::on_session_response(response_ptr resp, request_ptr req, client_session_ptr sess, client &owner)
{
    AsyncRequestContext *ctxt = dynamic_cast<AsyncRequestContext*>(req->get_context());
    if(ctxt)
        ctxt->OnResponse(resp, req, sess);
}

response_ptr OnvifRtspClient::on_session_request(request_ptr req, client_session_ptr sess, client &owner)
{
}

void OnvifRtspClient::on_session_request_timeout(request_ptr req, unsigned int millisec, client_session_ptr sess, client &owner)
{
}

void OnvifRtspClient::on_session_request_send_error(request_ptr req, const swift::error_code &ec, client_session_ptr sess, client &owner)
{
}

void OnvifRtspClient::on_session_teardown(client_session_ptr sess, client &owner)
{
    OnCloseSuccessed();
}


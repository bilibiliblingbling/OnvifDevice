#include "OnvifRtspContext.h"
#include "OnvifRtspClient.h"
#include <swift/net/sdp/sdp.hpp>
#include <swift/net/http/http.hpp>
AsyncRequestContext::AsyncRequestContext(OnvifRtspClient &client)
    : m_getReplyClient(client)
{
}

void AsyncRequestContext::OnResponse(response_ptr resp, request_ptr req)
{
}

void AsyncRequestContext::OnResponse(response_ptr resp, request_ptr req, client_session_ptr session)
{
}

void AsyncRequestContext::OnResponse(client_session_ptr session)
{
}

void AsyncRequestContext::OnHolderDestroy()
{
    delete this;
}

OptionContext::OptionContext(OnvifRtspClient &client)
    : AsyncRequestContext(client)
{
}

void OptionContext::OnResponse(response_ptr resp, request_ptr req, client_session_ptr session)
{
}

DescribeContext::DescribeContext(OnvifRtspClient &client)
    : AsyncRequestContext(client)
{
}

void DescribeContext::sendSetupRequests(const std::string &sdp)
{
    //swift::net::rtsp::rtsp_url setupUri;
    std::string setupUri;
    swift::net::rtsp::transport_value trans;
    std::string protoprofile;

    //swift::net::sdp::session_description::strict,
    swift::net::sdp::session_description desc(sdp);
    swift::net::sdp::media_list::const_iterator it = desc.get_medias().begin();
    for (; it != desc.get_medias().end(); ++it) {
        swift::net::sdp::media_description m = *it;
        if (m.media().type() == "video") {
            protoprofile = m.media().protocol();// RTP/AVP
            setupUri = m.get_attribute("control").value();

            size_t segPos = protoprofile.find("/");
            if (segPos != std::string::npos) {
                trans.set_protocol(protoprofile.substr(0,segPos));
                std::cout << protoprofile.substr(segPos);
                std::cout << setupUri;
                trans.set_profile(protoprofile.substr(segPos+1));
                std::cout << protoprofile.substr(segPos, protoprofile.length());
            } else {
                trans.set_protocol(protoprofile);
            }
            trans.set_client_port_param(m_getReplyClient.getLocalEndpoint().port());
            trans.set_unicast_param();
            m_getReplyClient.Setup(setupUri, trans);
        } else if (m.media().type() == "audio") {
            // TODO : 如果需要音频
        } else {
            // TODO : 如果需要其他
        }
    }
}

void DescribeContext::OnResponse(response_ptr resp, request_ptr req)
{
    if (resp->is_success()) {
        sendSetupRequests(resp->get_body());
    } else if (resp->get_status() == swift::net::http::HTTP_UNAUTHORIZED) {
        m_getReplyClient.SendAuthenticationRequest(req, resp, this);
    }
}

SetupContext::SetupContext(OnvifRtspClient &client)
    : AsyncRequestContext(client)
{
}

void SetupContext::OnResponse(response_ptr resp, request_ptr req, client_session_ptr session)
{
    if (resp->is_success()) {
        m_getReplyClient.SetSession(session);
        m_getReplyClient.Play();
        m_getReplyClient.OnStreamOpened();
    }
}

PlayContext::PlayContext(OnvifRtspClient &client)
    : AsyncRequestContext(client)
{
}

void PlayContext::OnResponse(response_ptr resp, request_ptr req, client_session_ptr session)
{
    if (resp->is_success()) {
        m_getReplyClient.GetStream()->OnOpened();
        m_getReplyClient.OnOpenSussiced();
    }
}

TeardownContext::TeardownContext(OnvifRtspClient &client)
    : AsyncRequestContext(client)
{
}

void TeardownContext::OnResponse(response_ptr resp, request_ptr req, client_session_ptr session)
{
    m_getReplyClient.GetStream()->OnClosed();
    m_getReplyClient.OnCloseSuccessed();
}


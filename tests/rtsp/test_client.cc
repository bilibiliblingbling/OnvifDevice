#include "test_client.h"

#include <swift/error_code.hpp>
#include <swift/net/rtsp/rtsp.hpp>
#include <swift/net/sdp/sdp.hpp>
#include <swift/crypto.hpp>
#include <swift/text/algorithm.hpp>

#define CONSOLE_DEBUG 1

#define DEFAULT_MULITCAST_ADDRESS "239.0.0.0"

/// 注册用 设备登陆帐号密码
#define USERNAME "admin"
#define PASSWORD "bq111111"

///
#define DEVICE_ADRESS "192.168.5.63"
#define DEVICE_SERVICE_PROT 554
/// Descirpt的回复消息中 解析sdp规范获得
#define SETUP_VIDEO_URI "rtsp://192.168.5.63:554/Streaming/Channels/101/trackID=1?transportmode=mcast&profile=Profile_1"

/// Uri信息:设备地址及端口|流信息-主流/辅流|传输模式-单播/组播|使用的配置文件
#define MULTICAST_URI "rtsp://192.168.5.63:554/Streaming/Channels/101?transportmode=mcast&profile=Profile_1"
#define UNICAST_URI   "rtsp://192.168.5.63:554/Streaming/Channels/101?transportmode=unicast&profile=Profile_1"

using namespace std;

#ifdef SWIFT_OLD
// TODO : digest部分并未实现;
MyAuthenCtxtptr MyAuthenication::makeAuthenContext(swift::net::rtsp::response_ptr resp, swift::net::http::_authenication_mode prior_mode)
{
    SWIFT_ASSERT(resp);
    if(resp->get_status() != swift::net::http::HTTP_UNAUTHORIZED)
    {
        return MyAuthenCtxtptr();
    }

    swift::net::hlp_core::const_header_ptr header = resp->get_header("WWW-Authenticate");
    if(!header)
    {
        return MyAuthenCtxtptr();
    }

    MyAuthenCtxtptr ctxt(new MyAuthenCtxt());
    if(!initAuthenCtxt(ctxt, header, prior_mode))
    {
        return MyAuthenCtxtptr();
    }

    return ctxt;
}

bool MyAuthenication::initAuthenCtxt(MyAuthenCtxtptr ctxt, const swift::net::hlp_core::const_header_ptr &authen_header, swift::net::http::_authenication_mode prior_mode)
{
    std::list<swift::net::hlp_core::header_value> vals = authen_header->get_values();
    std::list<swift::net::hlp_core::header_value>::const_iterator it = vals.begin();

    ctxt->m_mode = swift::net::http::authenication_any;
    std::string authen_argus;

    for(; it != vals.end(); ++it)
    {
        std::string authenline = swift::text::trim_copy((*it).to_string());
        std::string::size_type pos = authenline.find_first_of(' ');
        if(pos == std::string::npos)
        {
            continue;
        }

        std::string str = authenline.substr(0, pos);
        swift::net::http::_authenication_mode mode;
        if(swift::text::iequals(str, "Basic"))
        {
            mode = swift::net::http::authenication_basic;
        }
        else if(swift::text::iequals(str, "Digest"))
        {
            mode = swift::net::http::authenication_digest;
        }
        else
        {
            continue;
        }

        if(ctxt->m_mode == swift::net::http::authenication_any ||
            mode == prior_mode)
        {
            authen_argus = authenline.substr(pos, authenline.length() - pos);
            swift::text::trim(authen_argus);
            ctxt->m_mode = mode;
            if(mode == prior_mode)
            {
                break;
            }
        }
    }

    if(ctxt->m_mode == swift::net::http::authenication_any)
    {
        return false;
    }

    typedef std::map<swift::text::istring, std::string> params_map;
    params_map psmap;

    std::string::size_type pos = 0;
    int i = 0;
    for(int i = 0; i <= (int)authen_argus.length(); i++)
    {
        if(i == authen_argus.length() || authen_argus[i] == ',')
        {
            if(pos == i)
            {
                pos++;
                continue;
            }

            std::string::iterator iter = swift::text::find_char_between('=', authen_argus.begin() + pos, authen_argus.begin() + i - 1);
            if(iter != authen_argus.end())
            {
                int seqpos = (int)(iter - authen_argus.begin());
                std::string name = authen_argus.substr(pos, seqpos - pos);
                swift::text::trim(name);
                if(!name.empty())
                {
                    std::string value = authen_argus.substr(seqpos + 1, i - seqpos - 1);
                    swift::text::trim(value);
                    int length = (int)value.length();
                    if(length > 0)
                    {
                        int bgn = 0;
                        if(value[0] == '\"')
                        {
                            bgn = 1;
                            length--;
                        }
                        if(length > 0)
                        {
                            if(value[value.length() - 1] == '\"')
                            {
                                length--;
                            }
                        }
                        if(length != value.length())
                        {
                            value = value.substr(bgn, length);
                        }
                    }

                    psmap.insert(params_map::value_type(name, value));
                }
            }

            pos = i + 1;
        }
    }

    params_map::const_iterator iter = psmap.find("realm");
    if(iter != psmap.end())
    {
        ctxt->m_realm = (*iter).second;
    }


    if(ctxt->m_mode == swift::net::http::authenication_basic)
    {
        return true;
    }

    iter = psmap.find("nonce");
    if(iter != psmap.end())
    {
        ctxt->m_nonce = (*iter).second;
    }

    iter = psmap.find("algorithm");
    if(iter != psmap.end())
    {
        ctxt->m_algorithm = (*iter).second;
    }

    iter = psmap.find("qop");
    if(iter != psmap.end())
    {
        ctxt->m_qop = (*iter).second;
    }

    if(!ctxt->m_algorithm.empty() &&
        !swift::text::iequals(ctxt->m_algorithm, "MD5"))
    {
        return false;
    }
    if(!swift::text::iequals(ctxt->m_qop, "auth"))
    {
        return false;
    }

    ctxt->m_nc = 1;
    //ctxt->m_cnonce = generate_nonce();
    return true;
}

void MyAuthenCtxt::fillRequest(swift::net::rtsp::request_ptr req)
{
    SWIFT_ASSERT(req);

    swift::octets authen;
    if(m_mode == swift::net::http::authenication_basic)
    {
        authen = "Basic ";
        swift::octets src = m_username + ":" + m_pwd;
        authen += swift::crypto::base64::encode(swift::const_buffer(src.c_str(), src.length()));
    }
    req->set_header("Authorization", authen);
}
#endif

asyncRequestCtxt::asyncRequestCtxt(MyRtspClient &myClient)
    : m_myClient(myClient)
    , m_bAccessdenied(false)
{
}

void asyncRequestCtxt::onResponse(swift::net::rtsp::response_ptr resp, swift::net::rtsp::request_ptr req)
{
}

void asyncRequestCtxt::onResponse(swift::net::rtsp::response_ptr resp, swift::net::rtsp::request_ptr req, swift::net::rtsp::client_session_ptr sess)
{
}

void asyncRequestCtxt::on_holder_destroy()
{
    delete this;
}

descCtext::descCtext(MyRtspClient &myClient)
    : asyncRequestCtxt(myClient)
{

}

void descCtext::onResponse(swift::net::rtsp::response_ptr resp, swift::net::rtsp::request_ptr req)
{
    if (resp->is_success()) {
        cout << "[Describe Response：]\n" << resp->to_string() << endl;
#ifdef MULTICAST_MODEL
    swift::net::rtsp::rtsp_url url(MULTICAST_URI);
    swift::net::rtsp::transport_value trans("RTP/AVP;multicast;prot=4567-4568;destination=239.0.0.0");
#else
    swift::net::rtsp::rtsp_url url(UNICAST_URI);
    swift::net::rtsp::transport_value trans("RTP/AVP;unicast;client_port=43666-43667");
#endif
        m_myClient.m_client.setup(url, trans, new setupCtxt(m_myClient));
        cout << "[Send Setup Request........]" << __LINE__ << __func__ << endl;
    } else if (resp->get_status() == swift::net::rtsp::RTSP_UNAUTHORIZED && !m_bAccessdenied) {
        m_bAccessdenied = true;
        m_myClient.sendSafeAuthenticationRequest(resp, req, this);
        cout << "[Describe error: Reslut: UNAUTHORIZED code:" << swift::net::rtsp::RTSP_UNAUTHORIZED << "]" << endl;
    }

}

setupCtxt::setupCtxt(MyRtspClient &myClient)
    : asyncRequestCtxt(myClient)
{

}

void setupCtxt::onResponse(swift::net::rtsp::response_ptr resp, swift::net::rtsp::request_ptr req, swift::net::rtsp::client_session_ptr sess)
{
    if (!resp->is_success())
        return;

    cout << "[Setup Response:]\n" << resp->to_string() << endl;

    ///SETUP消息回复成功, 准备接收数据
#ifdef MULTICAST_MODEL
    m_myClient.m_socket.open();
    m_myClient.m_socket.join_group(DEFAULT_MULITCAST_ADDRESS);
#else
    m_myClient.m_receiver.start_receive();
#endif
    /// Arg Type|start|end
    swift::net::rtsp::range_value range("ntp", "0.000", "");
    cout << "[Send Play Request........]" << __LINE__ << __func__ << endl;
    m_myClient.m_session->play(range);
}

playCtxt::playCtxt(MyRtspClient &myClient)
    : asyncRequestCtxt(myClient)
{

}

void playCtxt::onResponse(swift::net::rtsp::response_ptr resp, swift::net::rtsp::request_ptr req, swift::net::rtsp::client_session_ptr sess)
{
    if (!resp->is_success())
        return;
    cout << "[Play Response:]\n" << resp->to_string() << endl;
}

teardownCtxt::teardownCtxt(MyRtspClient &myClient)
    : asyncRequestCtxt(myClient)
{

}

void teardownCtxt::onResponse(swift::net::rtsp::response_ptr resp, swift::net::rtsp::request_ptr req, swift::net::rtsp::client_session_ptr sess)
{
    cout << "[Teardown Response:]\n" << resp->to_string() << endl;
}

MyRtspClient::MyRtspClient(swift::engine &engine)
    : m_client(swift::net::ip::endpoint(DEVICE_ADRESS, DEVICE_SERVICE_PROT), *this, engine)
#ifdef MULTICAST_MODEL
    , m_socket(engine)
#else
    , m_receiver(swift::net::ip::endpoint(swift::net::ip::any_address_v4, info->m_listenPort), *this, engine)
#endif
{

}

void MyRtspClient::start()
{
    ///rtsp://192.168.5.63:554/Streaming/Channels/101xxx   application/sdp
#ifdef MULTICAST_MODEL
    swift::net::rtsp::rtsp_url url(MULTICAST_URI);
#else
    swift::net::rtsp::rtsp_url url(UNICAST_URI);
#endif

    cout << "[Send Describe Request........]" << __LINE__ << __func__ << endl;
    m_client.describe(url, "application/sdp", new descCtext(*this));
}

void MyRtspClient::stop()
{
    cout << "[Send Stop Request........]" << __LINE__ << __func__ << endl;
    if (m_session)
        m_session->teardown();
}

void MyRtspClient::sendSafeAuthenticationRequest(swift::net::rtsp::response_ptr resp, swift::net::rtsp::request_ptr req, swift::context *ctxt)
{
#ifdef SWIFT_OLD
    MyAuthenCtxtptr authen = MyAuthenication::makeAuthenContext(resp, swift::net::http::authenication_basic);
    if (authen) {
        authen->set_authenication(USERNAME, PASSWORD);
        authen->fillRequest(req);
        req->remove_header(swift::net::rtsp::h_CSeq);
    }
#else
    swift::net::rtsp::authentication::auth_ctxt_ptr auth
            = swift::net::rtsp::authentication::make_authentication_context(resp);
    if (auth) {
        auth->set_authentication(USERNAME, PASSWORD);
        auth->fill_request(req);
        req->remove_header(swift::net::rtsp::h_CSeq);
    }
#endif

    cout << "[Send Discribe Request with Authenication.........]" << __LINE__ << __func__ << endl;
	if (ctxt)
        m_client.send_request(req);
}

void MyRtspClient::on_rtp_packet(swift::net::rtp::rtp_packet &packet, const swift::net::ip::endpoint &remoteEP, swift::net::rtp::receiver &owner)
{
    std::cout << "[Recv CALLBACK]" << __LINE__ << __func__ << "receive : [pt][" << packet.get_payload_type() << "]"
         << "[pl][" << packet.get_payload_length() << "]"
         << "[timestamp][" << packet.get_timestamp() << "]"
         << "[sequence][" << packet.get_sequence() << "]" << std::endl;
}

void MyRtspClient::on_response(swift::net::rtsp::response_ptr resp, swift::net::rtsp::request_ptr req, swift::net::rtsp::client &owner)
{
    cout << "[CLIENT CALLBACK]" << __LINE__ <<__func__ << endl;
    cout << resp->to_string() << endl;
    asyncRequestCtxt *ctxt = dynamic_cast<asyncRequestCtxt*>(req->get_context());
    if (ctxt)
        ctxt->onResponse(resp, req);
}

swift::net::rtsp::response_ptr MyRtspClient::on_request(swift::net::rtsp::request_ptr req, swift::net::rtsp::client &owner)
{
    cout << "[CLIENT CALLBACK]" << __LINE__ <<__func__ << endl;
    cout << req->to_string() << endl;
    return swift::net::rtsp::response_ptr();
}

void MyRtspClient::on_request_timeout(swift::net::rtsp::request_ptr req, unsigned int millisec, swift::net::rtsp::client &owner)
{
    cout << "[CLIENT CALLBACK]" << __LINE__<< __func__ << endl;
    cout << req->to_string() << endl;
}

void MyRtspClient::on_request_send_error(swift::net::rtsp::request_ptr req, const swift::error_code &ec, swift::net::rtsp::client &owner)
{
    cout << "[CLIENT CALLBACK]" << __LINE__ << __func__ << endl;
    cout << req->to_string() << endl;
}

void MyRtspClient::on_session_setup(swift::net::rtsp::client_session_ptr sess, swift::net::rtsp::request_ptr req, swift::net::rtsp::response_ptr resp, swift::net::rtsp::client &owner)
{
    if (!resp->is_success())
        return;

    cout << "[CLIENT CALLBACK]" << __LINE__ <<  __func__ << endl;
    cout << "[session][" << sess->get_id() << endl;
    m_session = sess;
}

void MyRtspClient::on_session_response(swift::net::rtsp::response_ptr resp, swift::net::rtsp::request_ptr req, swift::net::rtsp::client_session_ptr sess, swift::net::rtsp::client &owner)
{
    ///每次当前Session回复消息时都会回调
    cout << "[CLIENT CALLBACK]" << __LINE__ << __func__ << endl;
    cout << "[session][" << sess->get_id() << endl;
    cout << resp->to_string() << endl;
    asyncRequestCtxt *ctxt = dynamic_cast<asyncRequestCtxt*>(req->get_context());
    if(ctxt)
        ctxt->onResponse(resp, req, sess);
}

swift::net::rtsp::response_ptr MyRtspClient::on_session_request(swift::net::rtsp::request_ptr req, swift::net::rtsp::client_session_ptr sess, swift::net::rtsp::client &owner)
{
    cout << "[CLIENT CALLBACK]" << __LINE__ << __func__ << endl;
    cout << "[session][" << sess->get_id() << endl;
    cout << req->to_string() << endl;
    return swift::net::rtsp::response_ptr();
}

void MyRtspClient::on_session_request_timeout(swift::net::rtsp::request_ptr req, unsigned int millisec, swift::net::rtsp::client_session_ptr sess, swift::net::rtsp::client &owner)
{
}

void MyRtspClient::on_session_request_send_error(swift::net::rtsp::request_ptr req, const swift::error_code &ec, swift::net::rtsp::client_session_ptr sess, swift::net::rtsp::client &owner)
{
}

void MyRtspClient::on_session_teardown(swift::net::rtsp::client_session_ptr sess, swift::net::rtsp::client &owner)
{
    cout << "[CLIENT CALLBACK]" << __LINE__ << __func__ << endl;
    cout << "[session][" << sess->get_id() << endl;
}

void MyRtspClient::on_not_matched_response(swift::net::rtsp::response_ptr resp, swift::net::rtsp::client &owner)
{
}

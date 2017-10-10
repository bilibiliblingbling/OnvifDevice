#ifndef __TEST_CLIENT_H__
#define __TEST_CLIENT_H__

#include <swift/net/rtsp/rtsp.hpp>
#include <swift/net/rtp/rtp.hpp>
#include <swift/utils/context.hpp>
#include <swift/net/http/http_authenication.hpp>
#include <iostream>

/// 老版本swift就自己实现注册相关功能
#define SWIFT_OLD 1

/// 0单播|1组播
#define MULTICAST_MODEL 1

class MyRtspClient;

#ifdef SWIFT_OLD
class MyAuthenCtxt;
typedef swift::shared_ptr<MyAuthenCtxt> MyAuthenCtxtptr;

class MyAuthenication : public swift::net::http::authenication
{
public:
    static MyAuthenCtxtptr makeAuthenContext(swift::net::rtsp::response_ptr resp, swift::net::http::_authenication_mode prior_mode);
    static bool initAuthenCtxt(MyAuthenCtxtptr ctxt, const swift::net::hlp_core::const_header_ptr& authen_header,
                                         swift::net::http::_authenication_mode prior_mode);
};

class MyAuthenCtxt : public swift::net::http::authen_ctxt
{
public:
    void fillRequest(swift::net::rtsp::request_ptr req);

    friend class MyAuthenication;
};
#endif

class asyncRequestCtxt : public swift::context
{
public:
    asyncRequestCtxt(MyRtspClient &myClient);

    virtual void onResponse(swift::net::rtsp::response_ptr resp, swift::net::rtsp::request_ptr req);
    virtual void onResponse(swift::net::rtsp::response_ptr resp, swift::net::rtsp::request_ptr req, swift::net::rtsp::client_session_ptr sess);

protected:
    void on_holder_destroy();

protected:
    MyRtspClient &m_myClient;
    bool m_bAccessdenied;
};

class descCtext : public asyncRequestCtxt
{
public:
    descCtext(MyRtspClient &myClient);

    virtual void onResponse(swift::net::rtsp::response_ptr resp, swift::net::rtsp::request_ptr req);
};

class setupCtxt : public asyncRequestCtxt
{
public:
    setupCtxt(MyRtspClient &myClient);

    virtual void onResponse(swift::net::rtsp::response_ptr resp, swift::net::rtsp::request_ptr req, swift::net::rtsp::client_session_ptr sess);
};

class playCtxt : public asyncRequestCtxt
{
public:
    playCtxt(MyRtspClient &myClient);

    virtual void onResponse(swift::net::rtsp::response_ptr resp, swift::net::rtsp::request_ptr req, swift::net::rtsp::client_session_ptr sess);
};

class teardownCtxt : public asyncRequestCtxt
{
public:
    teardownCtxt(MyRtspClient &myClient);

    virtual void onResponse(swift::net::rtsp::response_ptr resp, swift::net::rtsp::request_ptr req, swift::net::rtsp::client_session_ptr sess);
};

class MyRtspClient : public swift::net::rtsp::client_callback, public swift::net::rtp::receiver_callback
{
public:
    MyRtspClient(swift::engine &engine);
public:
    void start();
    void stop();
public:
    swift::net::rtsp::client m_client;
    swift::net::rtsp::client_session_ptr m_session;

#ifdef MULTICAST_MODEL
    swift::net::ip::udp::socket m_socket;
#else
    swift::net::rtp::receiver m_receiver;
#endif

public:
    void sendSafeAuthenticationRequest(swift::net::rtsp::response_ptr resp, swift::net::rtsp::request_ptr req, swift::context *ctxt);
private:
    virtual void on_rtp_packet(swift::net::rtp::rtp_packet &packet, const swift::net::ip::endpoint &remoteEP, swift::net::rtp::receiver &owner);

private:
    virtual void on_response(swift::net::rtsp::response_ptr resp, swift::net::rtsp::request_ptr req, swift::net::rtsp::client &owner);
    virtual swift::net::rtsp::response_ptr on_request(swift::net::rtsp::request_ptr req, swift::net::rtsp::client &owner);
    virtual void on_request_timeout(swift::net::rtsp::request_ptr req, unsigned int millisec, swift::net::rtsp::client &owner);
    virtual void on_request_send_error(swift::net::rtsp::request_ptr req, const swift::error_code &ec, swift::net::rtsp::client &owner);
private:
    virtual void on_session_setup(swift::net::rtsp::client_session_ptr sess, swift::net::rtsp::request_ptr req, swift::net::rtsp::response_ptr resp, swift::net::rtsp::client &owner);    virtual void on_session_response(swift::net::rtsp::response_ptr resp, swift::net::rtsp::request_ptr req, swift::net::rtsp::client_session_ptr sess, swift::net::rtsp::client &owner);
    virtual swift::net::rtsp::response_ptr on_session_request(swift::net::rtsp::request_ptr req, swift::net::rtsp::client_session_ptr sess, swift::net::rtsp::client &owner);
    virtual void on_session_request_timeout(swift::net::rtsp::request_ptr req, unsigned int millisec, swift::net::rtsp::client_session_ptr sess, swift::net::rtsp::client &owner);    virtual void on_session_request_send_error(swift::net::rtsp::request_ptr req, const swift::error_code &ec, swift::net::rtsp::client_session_ptr sess, swift::net::rtsp::client &owner);
    virtual void on_session_teardown(swift::net::rtsp::client_session_ptr sess, swift::net::rtsp::client &owner );
    virtual void on_not_matched_response(swift::net::rtsp::response_ptr resp, swift::net::rtsp::client &owner);
};
#endif /// __TEST_CLIENT_H__

#include "OnvifBasic.h"

#include "soap/gsoap/plugin/wsseapi.h"


OnvifBasic::OnvifBasic(const string &username, const string &password, bool bIsConnect)
    : m_auth(username, password, SIGNATURE_ROLE)
    , m_isConnect(bIsConnect)
{
    InitSoapRelated();
    InitReqResp();
}

OnvifBasic::OnvifBasic(const SoapAuthentication& auth, bool bIsConnect)
    : m_auth(auth)
    , m_isConnect(bIsConnect)
{
    InitSoapRelated();
    InitReqResp();
}

OnvifBasic::~OnvifBasic()
{
    DestorySoap();
}

SoapAuthentication OnvifBasic::GetAuthInfo()
{
    return m_auth;
}

OnvifSoapResult OnvifBasic::TokenDigest()
{
    return soap_wsse_add_UsernameTokenDigest(m_soap, m_auth.m_signatureRole.c_str(), m_auth.m_username.c_str(), m_auth.m_password.c_str());
}

bool OnvifBasic::RemoteDetectDevice(const string &endpoint)
{
    OnvifSoapResult result = soap_send___wsdd__Probe(m_soap, endpoint.c_str(), NULL, m_probeReq);
    while(result == SOAP_OK) {
        result = soap_recv___wsdd__ProbeMatches(m_soap, m_probeResp);
        if(result == SOAP_OK) {
            if(m_soap->error) {
                m_isConnect = false;
                return false;
            } else {
                if (m_probeResp->wsdd__ProbeMatches != NULL) {
                    m_isConnect = true;
                    return true;
                }
            }
        }
    }
    return m_isConnect;
}

bool OnvifBasic::IsConnect()
{
    return m_isConnect;
}

void OnvifBasic::InitSoapRelated()
{
    m_soap = soap_new2(SOAP_C_UTFSTRING, SOAP_C_UTFSTRING);
    SWIFT_ASSERT(m_soap);
    m_soap->recv_timeout = 50;
    soap_set_namespaces(m_soap, namespaces);
}

void OnvifBasic::InitReqResp()
{
    m_probeReq = (ProbeReq*)soap_malloc(m_soap, sizeof(ProbeReq));
    m_probeResp = (ProbeResp*)soap_malloc(m_soap, sizeof(ProbeResp));
}

void OnvifBasic::DestorySoap()
{
    soap_destroy(m_soap);
    soap_end(m_soap);
    soap_free(m_soap);
}


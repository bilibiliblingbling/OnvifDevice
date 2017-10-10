#include "OnvifPTZ.h"
#include "soap/gsoap/plugin/wsseapi.h"
OnvifPTZ::OnvifPTZ(const string &ptzServername, SoapAuthentication auth, bool bIsConnect)
    : OnvifBasic(auth, bIsConnect)
    , m_ptzServerAddr(ptzServername)
{

}

OnvifPTZ::~OnvifPTZ()
{

}

OnvifSoapResult OnvifPTZ::ContinuousMove(char* profiletoken, PTZPantilt *pantilt, PTZZoom *zoom)
{
    soap_default__tptz__ContinuousMove(m_soap, m_move);
    m_move->ProfileToken = profiletoken;
    struct tt__PTZSpeed *speed = (struct tt__PTZSpeed*)soap_malloc(m_soap, sizeof(struct tt__PTZSpeed));
    speed->PanTilt = pantilt;
    speed->Zoom = zoom;
    m_move->Velocity = speed;
    TokenDigest();
    OnvifSoapResult result = soap_call___tptz__ContinuousMove(m_soap, m_ptzServerAddr.c_str(), NULL, m_move, m_moveResp);

    return result;
}

OnvifSoapResult OnvifPTZ::GotoHomePosition(char* profiletoken, PTZArg *defaultArg)
{
    m_gohome->ProfileToken = profiletoken;
    m_gohome->Speed = defaultArg;

    TokenDigest();
    OnvifSoapResult result = soap_call___tptz__GotoHomePosition(m_soap, m_ptzServerAddr.c_str(), NULL, m_gohome, m_gohomeResp);
    return result;
}

void OnvifPTZ::InitReqResp()
{
    m_move = (PTZMoveReq*)soap_malloc(m_soap, sizeof(PTZMoveReq));
    m_moveResp = (PTZMoveResp*)soap_malloc(m_soap, sizeof(PTZMoveResp));
    m_ptzArg = (PTZArg*)soap_malloc(m_soap, sizeof(PTZArg));

    m_gohome = (GotoHomePos*)soap_malloc(m_soap, sizeof(GotoHomePos));
    m_gohomeResp = (GotoHomePosResp*)soap_malloc(m_soap, sizeof(GotoHomePosResp));
}

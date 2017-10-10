#ifndef __ONVIF_DEVICE_PLUGIN_ONVIF_PTZ_H__
#define __ONVIF_DEVICE_PLUGIN_ONVIF_PTZ_H__

#include "OnvifBasic.h"

class OnvifPTZ : public OnvifBasic
{
public:
    OnvifPTZ(const string& ptzServername, SoapAuthentication auth, bool bIsConnect);
    ~OnvifPTZ();

public:
    OnvifSoapResult ContinuousMove(char *profiletoken, PTZPantilt *pantilt, PTZZoom *zoom);
    OnvifSoapResult GotoHomePosition(char *profiletoken, PTZArg *defaultArg);

private:
    std::string m_ptzServerAddr;

    PTZArg *m_ptzArg;
    PTZMoveReq *m_move;
    PTZMoveResp *m_moveResp;

    GotoHomePos *m_gohome;
    GotoHomePosResp *m_gohomeResp;

private:
    void InitReqResp();
};

typedef swift::shared_ptr<OnvifPTZ> OnvifPTZPtr;

#endif

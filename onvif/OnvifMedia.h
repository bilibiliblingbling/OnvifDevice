#ifndef __ONVIF_DEVICE_PLUGIN_ONVIF_ONVIFMEDIA_H__
#define __ONVIF_DEVICE_PLUGIN_ONVIF_ONVIFMEDIA_H__

#include "OnvifBasic.h"
#include "../OnvifDeviceConfigure.h"

class OnvifMedia : public OnvifBasic
{
public:
    OnvifMedia(const string& mediaServername, SoapAuthentication auth, bool bIsConnect);
    ~OnvifMedia();

public:
    // profile
    ProfilesResp *GetMediaProfiles();

    // playURI
    const string GetStreamURI(int streamID);

    // multicast
    OnvifSoapResult StartMulticast(bool bIsMainStream = true);
    OnvifSoapResult StopMulticast(bool bIsMainStream = true);
    OnvifSoapResult SetMulticastEncoderConfig(unsigned long multAddr, unsigned short destPort, bool bIsMainStream = true);

    // videoconfig
    OnvifSoapResult SetVideoEncoderConfig(OnvifStreamArguments &configArgs, bool bIsMainStream = true);

    // osd
    GetOSDsConfigResp *GetOSDs(bool bIsMainStream = true);
    OnvifSoapResult SetOSD(OnvifOSDArguments *configArgs, bool bIsMainStream ,bool bIsDateTimeSetting = true);
    OnvifSoapResult CreateOSD(OnvifOSDArguments *configArgs, bool bIsMainStream, bool bIsDateTimeSetting = true);
    OnvifSoapResult DeleteOSD(OnvifOSDArguments *configArgs, bool bIsMainStream, bool bIsDateTimeSetting = true);

private:
    OnvifSoapResult CreateSubStreamMediaSourceConfigure();
    OnvifSoapResult GetProfiles();

private:
    ProfilesReq                 *m_getProfilesReq;
    ProfilesResp                *m_getProfilesResp;

    StreamUriReq                *m_getStreamUriReq;
    StreamUriResp               *m_getStreamUriResp;

    StartMulticastReq           *m_startMulticastReq;
    StartMulticastResp          *m_startMulticastResp;
    StopMulticastReq            *m_stopMulticastReq;
    StopMulticastResp           *m_stopMulticastResp;

    GetOSDsConfig               *m_getOSDsReq;
    GetOSDsConfigResp           *m_getOSDsResp;
    SetOSDConfig                *m_setOSDReq;
    SetOSDConfigResp            *m_setOSDResp;
    CreateOSDConfig             *m_createOSDReq;
    CreateOSDConfigResp         *m_createOSDResp;
    DeleteOSDConfig             *m_deleteOSDReq;
    DeleteOSDConfigResp         *m_deleteOSDResp;

    SetVideoEncoderConfigReq    *m_setConfigOptReq;
    SetVideoEncoderConfigResp   *m_setConfigOptResp;

    AddVideoSource              *m_addVideoSourceReq;
    AddVideoSourceResp          *m_addVideoSourceResp;

    std::string m_mediaServerAddr;

private:
    void InitReqResp();
};

typedef swift::shared_ptr<OnvifMedia> OnvifMediaPtr;

#endif

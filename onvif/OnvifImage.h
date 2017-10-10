#ifndef __ONVIF_DEVICE_PLUGIN_ONVIF_ONVIFIMAGE_H__
#define __ONVIF_DEVICE_PLUGIN_ONVIF_ONVIFINAGE_H__

#include "OnvifBasic.h"
#include "../OnvifDeviceConfigure.h"

class OnvifImage;
typedef swift::shared_ptr<OnvifImage> OnvifImagePtr;

class OnvifImage : public OnvifBasic
{
public:
    OnvifImage(const string& imageServername, SoapAuthentication auth, bool bIsConnect);

public:
    GetImagingResp *GetImagingSetting(char* token);
    OnvifSoapResult SetImagingSetting(char* token, OnvifStreamArguments &args);

// TODO : PTZ func about focus iris ...
private:
    GetImagingReq *m_getImage;
    GetImagingResp *m_getImageResp;

    SetImagingReq *m_setImage;
    SetImagingResp *m_setImageResp;

    string m_imageServerAddr;
private:
    void InitReqResp();
};

#endif



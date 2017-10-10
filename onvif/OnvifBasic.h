#ifndef __ONVIF_DEVICE_PLUGIN_ONVIF_ONVIFBASIC_H__
#define __ONVIF_DEVICE_PLUGIN_ONVIF_ONVIFBASIC_H__

#include "OnvifDef.h"
#include "../OnvifDeviceHelper.h"

class OnvifBasic
{
public:
    OnvifBasic(const string& username, const string& password, bool bIsConnect = false);
    OnvifBasic(const SoapAuthentication &auth, bool bIsConnect = false);
    virtual ~OnvifBasic();

public:
    SoapAuthentication GetAuthInfo();
    bool IsConnect();

protected:
    // WS-Security 通用鉴权接口
    OnvifSoapResult TokenDigest();
    // WS-Discovery 提供综合的发现设备接口(设备地址，和组播地址)
    bool RemoteDetectDevice(const string &endpoint);
protected:
    // soap 通用结构
    OnvifSoap *m_soap;

    // WS-Security plugin 参数需求
    SoapAuthentication m_auth;

    // WS-Discovery plugin
    ProbeReq *m_probeReq;
    ProbeResp *m_probeResp;

    bool m_isConnect;
private:
    void InitSoapRelated();
    void InitReqResp();
    void DestorySoap();
};

#endif ///__ONVIF_DEVICE_PLUGIN_ONVIF_ONVIFDEVICE_H__


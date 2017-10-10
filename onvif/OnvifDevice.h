#ifndef __ONVIF_DEVICE_PLUGIN_ONVIF_ONVIFDEVICE_H__
#define __ONVIF_DEVICE_PLUGIN_ONVIF_ONVIFDEVICE_H__

#include "OnvifBasic.h"

class OnvifDevice;
typedef swift::shared_ptr<OnvifDevice> OnvifDevicePtr;

class OnvifDevice : public OnvifBasic
{
public:
    OnvifDevice(const std::string &deviceIpAddr, const std::string &username, const std::string &password);
    ~OnvifDevice();

public:
    // for create subServer
    const string GetSubServerAddress(const string &subserverdef);

    bool ConnectDevice(); // for connector
    bool DisconnectDevice(); // for control

protected:
    // get subServers name
    ServicesReq *m_getServicesReq;
    ServicesResp *m_getServicesResp;

    // 服务地址: xxx.xxx.xxx.xxx/Onvif/device_servers
    string m_deviceServerAddr;
private:
    // Detection
    void SetOnvifDevice(const string &deviceAddr);
    void InitReqResp();
};


#endif /// __ONVIF_DEVICE_PLUGIN_ONVIF_ONVIFDEVICE_H__

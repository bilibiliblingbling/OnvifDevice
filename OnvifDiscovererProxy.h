#ifndef __ONVIF_DEVICE_PLUGIN_DISCOVERER_PROXY_H__
#define __ONVIF_DEVICE_PLUGIN_DISCOVERER_PROXY_H__

#include "../Utils/include/DiscovererProxy.h"
#include "onvif/OnvifDiscovery.h"

class OnvifDiscovererProxyBase : public DiscovererProxy
{
protected:
    OnvifDiscovererProxyBase();

    void OnDeviceFound(__wsdd__ProbeMatches *pResp);
    static DevBaseInfo GenerateDeviceInfo(__wsdd__ProbeMatches *pResp);

    // DiscovererProxy interface
protected:
    bool DoStart(){}
    void DoStop(){}
    bool DoRefresh(){}
};

////////////////////////////////////////////////////////////////////////////////

class OnvifDiscovererProxy : public OnvifDiscovererProxyBase
{
public:
    static OnvifDiscovererProxy& Instance();

    ~OnvifDiscovererProxy();
private:
    OnvifDiscovererProxy();

protected:
    virtual bool DoStart();
    virtual void DoStop();
    virtual bool DoRefresh();
};

#endif ///__ONVIF_HIK_DEVICE_PLUGIN_DISCOVERER_PROXY_H__

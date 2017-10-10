#include "OnvifDiscovererProxy.h"

OnvifDiscovererProxyBase::OnvifDiscovererProxyBase()
{

}

void OnvifDiscovererProxyBase::OnDeviceFound(__wsdd__ProbeMatches *pResp)
{
    DevBaseInfo devInfo = OnvifDiscovererProxyBase::GenerateDeviceInfo(pResp);

    if (devInfo.IsValid())
        DiscovererProxy::OnDeviceFound(devInfo);
}

DeviceLayer::DevBaseInfo OnvifDiscovererProxyBase::GenerateDeviceInfo(__wsdd__ProbeMatches *pResp)
{
}

///////////////////////////////////////////////////////////////////////////////

OnvifDiscovererProxy &OnvifDiscovererProxy::Instance()
{
    static OnvifDiscovererProxy s_instance;
    return s_instance;
}

OnvifDiscovererProxy::~OnvifDiscovererProxy()
{
}

OnvifDiscovererProxy::OnvifDiscovererProxy()
{
}

bool OnvifDiscovererProxy::DoStart()
{
}

void OnvifDiscovererProxy::DoStop()
{
}

bool OnvifDiscovererProxy::DoRefresh()
{
    DoStop();
    DoStart();
}

#ifndef __ONVIF_DEVICE_PLUGIN_DISCOVERER_H__
#define __ONVIF_DEVICE_PLUGIN_DISCOVERER_H__

#include "../Utils/include/DiscovererProxy.h"

class OnvifDiscoverer : public ProxyedDiscovererBase
{
public:
    OnvifDiscoverer(const DiscoverParams& params, const DLPCreateClassContextPtr& pCreateClassContext);

protected:
    virtual DiscovererProxy& GetProxy();
};

#endif ///__ONVIF_DEVICE_PLUGIN_DISCOVERER_H__

#include "OnvifDiscoverer.h"
#include "OnvifDiscovererProxy.h"

OnvifDiscoverer::OnvifDiscoverer(const DeviceLayer::DiscoverParams &params, const DLPCreateClassContextPtr &pCreateClassContext)
    : ProxyedDiscovererBase(params, pCreateClassContext)
{

}

DiscovererProxy &OnvifDiscoverer::GetProxy()
{
    return OnvifDiscovererProxy::Instance() ;
}

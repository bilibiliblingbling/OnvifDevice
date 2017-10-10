#ifndef __ONVIF_DEVICE_PLUGIN_ONVIF_ONVIFDISCOVERY_H__
#define __ONVIF_DEVICE_PLUGIN_ONVIF_ONVIFDISCOVERY_H__

#include "OnvifBasic.h"

class OnvifDiscovery : public OnvifBasic
{
public:
    OnvifDiscovery(const string& usrname, const string& password);
    ~OnvifDiscovery();

    int RemoteDiscovery();

    // OnvifDevice interface
private:
};

typedef swift::shared_ptr<OnvifDiscovery> OnvifDiscoveryPtr;
#endif

#ifndef __ONVIF_DEVICE_PLUGIN_CLASS_H__
#define __ONVIF_DEVICE_PLUGIN_CLASS_H__

#include <DeviceLayer/ObjectPtr.h>
#include <DeviceLayer/ObjectInterfaceDef.h>
#include <DeviceLayer/ObjectInterface.h>

#include "../Utils/include/ObjectUtils.h"

class OnvifDeviceClass : public DLPDeviceClassEx <OnvifDeviceClass>
{
public:
    OnvifDeviceClass(IDLPCreateClassContext* pCreateContext);
    ~OnvifDeviceClass();

public:
    virtual DP_RESULT DLPMETHOD CreateConnector(IDLPConnector** ppConnector);
    virtual DP_RESULT DLPMETHOD CreateDiscoverer(const DiscoverParams& params, IDLPDiscoverer **ppDiscoverer);

    DECLARE_DLPDEVICECLASS_INIT_FUNCS(OnvifDeviceClass)
};

#endif ///__ONVIF_DEVICE_PLUGIN_CLASS_H__

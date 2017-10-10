#include "OnvifDeviceClass.h"
#include "OnvifDiscoverer.h"
#include "OnvifConnector.h"

DEFINE_DLPDEVICECLASS_GLOBALS(OnvifDeviceClass, "HikDevice", DLPDCIS_ENGINE_INSTANCE)
//DEFINE_DLPDEVICECLASS_GLOBALS(OnvifDeviceClass, "PanSCDevice", DLPDCIS_ALL)//DLPDCIS_ENGINE_INSTANCE)

OnvifDeviceClass::OnvifDeviceClass(IDLPCreateClassContext *pCreateContext)
    : DLPDeviceClassEx<OnvifDeviceClass>(pCreateContext)
{
}

OnvifDeviceClass::~OnvifDeviceClass()
{
}

DP_RESULT OnvifDeviceClass::CreateConnector(IDLPConnector **ppConnector)
{
    if (!ppConnector)
        return DeviceLayer::DLR_E_INVALID_PARAM;

    *ppConnector = new OnvifConnector(m_pCreateContext);

    return DeviceLayer::DLR_S_OK;
}

DP_RESULT OnvifDeviceClass::CreateDiscoverer(const DeviceLayer::DiscoverParams &params, IDLPDiscoverer **ppDiscoverer)
{
    if(!ppDiscoverer)
    {
        return DeviceLayer::DLR_E_INVALID_PARAM;
    }

    *ppDiscoverer = new OnvifDiscoverer(params, m_pCreateContext);
    return DeviceLayer::DLR_S_OK;
}

void OnvifDeviceClass::InitializeClass()
{
    DLPDeviceClassEx<OnvifDeviceClass>::InitializeClass();
}

void OnvifDeviceClass::UninitializeClass()
{
    DLPDeviceClassEx<OnvifDeviceClass>::UninitializeClass();
}

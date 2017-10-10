#include "OnvifConnector.h"

#include <uuid/uuid.h>
#include <swift/thread/thread.hpp>

#include "OnvifDeviceControl.h"

#include "onvif/OnvifDevice.h"


OnvifConnector::OnvifConnector(const DLPCreateClassContextPtr &pCreateClassContext)
    : m_pCreateClassContext(pCreateClassContext)
{
}

OnvifConnector::~OnvifConnector()
{
}

DP_RESULT OnvifConnector::Connect(const DeviceLayer::DevBaseInfo &info, IDLPContext *pContext)
{
    if (!info.IsValid())
        return DeviceLayer::DLR_E_INVALID_PARAM;

    if (info.GetConnectionMode() != DeviceLayer::CONN_MODE_CONNECT)
        return DeviceLayer::DLR_E_INVALID_PARAM;

    const char* temp = info.GetConnectionArgument("IPAddress");
    std::string IPAddr = temp ? temp : "" ;
    if (IPAddr.empty())
        return DeviceLayer::DLR_E_INVALID_PARAM;

    temp = info.GetConnectionArgument("Username");
    std::string username = temp ? temp : "";

    temp = info.GetConnectionArgument("Password");
    std::string password = temp ? temp : "";
    OnvifDeviceHelper::PostHandler(swift::bind(&OnvifConnector::ConnectProxy, OnvifConnectorPtr(this, true),
                                               IPAddr, username, password, pContext));

    return DeviceLayer::DLR_S_OK;
}

void OnvifConnector::ConnectProxy(const OnvifConnectorPtr &pConnector, const std::string &addr, const std::string &username, const std::string &password, IDLPContext *pContext)
{
    if (pConnector) {
        pConnector->DoConnect(addr, username, password, pContext);
    }
}

void OnvifConnector::DoConnect(const std::string &addr, const std::string &username, const std::string &password, IDLPContext *pContext)
{
    IDLPConnectorCallback *pCallback = GetSafeConnectorCallback();
    if (pCallback) {
        OnvifDevicePtr onvifdevice(new OnvifDevice(addr, username, password));
        if (onvifdevice->ConnectDevice()) {
            OnvifDeviceControlPtr pDevCtrl(new OnvifDeviceControl(onvifdevice, m_pCreateClassContext));
            pCallback->OnConnectSuccess(static_cast<IDLPDeviceControl*>(pDevCtrl.GetPtr(false)),
                                        static_cast<IDLPConnector*>(this), pContext);
        } else {
            pCallback->OnConnectFailed(DeviceLayer::DLR_E_FAIL, static_cast<IDLPConnector*>(this), pContext);
        }
        pCallback->Release();
    }
}

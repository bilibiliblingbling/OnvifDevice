#ifndef __ONVIF_DEVICE_PLUGIN_CONNECTOR_H__
#define __ONVIF_DEVICE_PLUGIN_CONNECTOR_H__

#include <DeviceLayer/ObjectPtr.h>
#include <DeviceLayer/ObjectInterfaceDef.h>
#include <DeviceLayer/ObjectInterface.h>

#include "../Utils/include/ObjectUtils.h"

class OnvifConnector;
typedef DeviceLayer::ObjectPtr<OnvifConnector>	OnvifConnectorPtr;

class OnvifConnector : public DLPConnector
{
public:
    OnvifConnector(const DLPCreateClassContextPtr& pCreateClassContext);
    ~OnvifConnector();

    virtual DP_RESULT DLPMETHOD Connect(const DevBaseInfo& info, IDLPContext* pContext);

    DEFINE_OBJECT_INTERFACE()

protected:
    static void ConnectProxy(const OnvifConnectorPtr& pConnector, const std::string& addr
                             , const std::string& username, const std::string& password, IDLPContext* pContext);

    void DoConnect(const std::string& addr, const std::string& username
                   , const std::string& password, IDLPContext* pContext);

private:
    DLPCreateClassContextPtr m_pCreateClassContext;
};

#endif ///__ONVIF_DEVICE_PLUGIN_CONNECTOR_H__

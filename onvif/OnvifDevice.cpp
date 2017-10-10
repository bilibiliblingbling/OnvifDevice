#include "OnvifDevice.h"

#include "soap/namespaces.h"

static const string& cutDeviceIpServers(string& serverAddresses)
{
    for(int i = 0; i <= serverAddresses.length(); i++) {
        if (serverAddresses[i] == ' ')
            serverAddresses.erase(i);
    }
    return serverAddresses;
}

OnvifDevice::OnvifDevice(const string &deviceIpAddr, const string &username, const string &password)
    : OnvifBasic(username, password)
{
    InitReqResp();
    SetOnvifDevice(deviceIpAddr);
}

OnvifDevice::~OnvifDevice()
{

}

void OnvifDevice::SetOnvifDevice(const string &deviceAddr)
{
    string endpoint = PROBE_ADDRESS_HEADER + deviceAddr + PROBE_ADDRESS_PORT;
    string addresses;
    if (RemoteDetectDevice(endpoint)) {
        addresses = m_probeResp->wsdd__ProbeMatches->ProbeMatch->XAddrs;
    }
    m_deviceServerAddr = cutDeviceIpServers(addresses);
    cout << "设备Onvif地址： " << m_deviceServerAddr << endl;
    return;
}

bool OnvifDevice::ConnectDevice()
{
#if 0
    if (m_deviceServerAddr.empty())
        return false;

    wsdd__HelloType hello;
    hello.wsa__EndpointReference = m_probeResp->wsdd__ProbeMatches->ProbeMatch->wsa__EndpointReference;
    hello.MetadataVersion = m_probeResp->wsdd__ProbeMatches->ProbeMatch->MetadataVersion;
    struct __wsdd__Hello helloResp;

    int result = soap_send___wsdd__Hello(m_soap, m_deviceServerAddr.c_str(), NULL, &hello);
    if (result == SOAP_OK)
        result = soap_recv___wsdd__Hello(m_soap, &helloResp);

    result ? m_isConnect = false : m_isConnect = true;
    return m_isConnect;
#else
    return true;
#endif
}

bool OnvifDevice::DisconnectDevice()
{
    wsdd__ByeType bye;
    struct __wsdd__Bye byeResp;
    bye.wsa__EndpointReference = m_probeResp->wsdd__ProbeMatches->ProbeMatch->wsa__EndpointReference;

    int result = soap_send___wsdd__Bye(m_soap, m_deviceServerAddr.c_str(), NULL, &bye);
    if (result == SOAP_OK)
        result = soap_recv___wsdd__Bye(m_soap, &byeResp);

    result ? m_isConnect = true : m_isConnect = false;
    return m_isConnect;
}

const string OnvifDevice::GetSubServerAddress(const string &subserverdef)
{
    m_getServicesReq->IncludeCapability = xsd__boolean__false_;

    OnvifSoapResult result = soap_call___tds__GetServices(m_soap, m_deviceServerAddr.c_str(), NULL, m_getServicesReq, m_getServicesResp);

    if (result == SOAP_OK) {
        int i;
        if (m_getServicesResp->Service != NULL) {
            for(i = 0; i < m_getServicesResp->__sizeService; i++) {
                //specification onvif的规范文档: ex:"http://www.onvif.org/ver10/media/wsdl"
                if (!strcmp(m_getServicesResp->Service[i].Namespace, subserverdef.c_str())) {
                    return m_getServicesResp->Service[i].XAddr;
                }
            }
        }
    }
    return NULL;
}

void OnvifDevice::InitReqResp()
{
    m_getServicesReq = (ServicesReq*)soap_malloc(m_soap, sizeof(ServicesReq));
    m_getServicesResp = (ServicesResp*)soap_malloc(m_soap, sizeof(ServicesResp));
}

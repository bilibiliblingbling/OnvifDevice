#include <stdlib.h> 
#include <string.h> 
#include <stdint.h>

#include "soapH.h"
#include "soapStub.h"
#include "wsdd.h"
#include <uuid/uuid.h>

//ONVIF multicast address at ws-discovery port
#define	MULTICAST_ADDRESS       "soap.udp://239.255.255.250:3702"

//ONVIF ws-security Anuthon
#define ONVIF_USER              "admin"
#define ONVIF_PASSWORD          "bq111111"

//Change DeviceIPAdress To
#define ONVIF_DIST_NET_WORK_NEW "192.168.0.66"

/*! onvif 基本流程 探索发现-获取设备及能力-常规业务逻辑
 *  无连接概念,通信请求都需要鉴权(少数获取参数请求除外),
 *  无明确流程,大多数请求消息失败不影响其他请求(发现设备获取能力配置等请求除外)
 *  无配置同步问题,修改配置前不需同步(修改设备IP地址后除外)
 *  需要使用现有配置的时，设备自己会拿profile,
 *  请求消息中只需要告诉设备拿哪一个配置文件即可,相关字段:ProfileToken;
 * SendProbe (向"发现设备"专有的组播地址中所有设备发送消息)
 *    |->probeMatches (支持onvif协议的设备 会给正确的回复)
 *        |->select a Matche Device (选中需要通信的设备)
 *           |   |  |->getcapabilities(获取设备能力) / GetMedia0/PTZ/AlarmServiceAddress...等等
 *           |   |--->getProfiles(获取配置)  	| / GetMedia/PTZ/AlarmProfiles...等等
 *           |                   |          |
 *           |   +—————————+     |          |
 *           |-->| Perform |<----|          |
 *               |  Tasks  |<---------------|
 *               +—————————+
 *        执行任务时根据不同的功能会需要设备的能力，配置，服务地址等，需要什么申请什么
 */

/*! 打开单播流程: Opt:Option  Req:Require  R1o:first[Req]-else[Opt]
 *[R1o]GetMediaServiceAddress(获取访问节点,如其他回复capabilities或probematches成功也包含Media->Xaddress字段,可直接取用,不修改设备IP此节点不会发生修改)
 *[R1o]   |-->GetMediaProfile(获取配置文件,推荐修改配置参数时优先获取该配置所在的profile)
 *[Opt]           |-->Select profile with H.264 Video encoder configuration(启用选定的配置，有些设备提供多项配置，本例可以忽略，详情看文档)
 *[Opt]                  |-->Setting video encoder configuration (如不需要修改配置,此步骤可忽略,直接使用getprofile中的profiletoken)
 *[Req]                      |-->GetStreamUri-->Rtsp......(获取视频流地址,相关流操作,交给Rtsp协议处理)
 */

// Init Soap , Fill Defalut Event Header;
struct soap* NewSoap(struct SOAP_ENV__Header *header,struct soap* soap, wsdd__ProbeType *req_, wsdd__ScopesType *sScope_)
{
    soap = soap_new();
    if(NULL == soap ) {
        printf("[%d][%s][Soap New Error]\n", __LINE__, __func__);
        return NULL;
    }

    soap->recv_timeout = 50;
    soap_set_namespaces(soap, namespaces);

    soap_default_SOAP_ENV__Header(soap, header);

    uuid_t uuid;
    char guid_string[100];
    uuid_generate(uuid);
    uuid_unparse(uuid, guid_string);

    header->wsa__MessageID = guid_string;
    header->wsa__To = "urn:schemas-xmlsoap-org:ws:2005:04:discovery";
    header->wsa__Action = "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe";
    soap->header = header;

    soap_default_wsdd__ScopesType(soap, sScope_);
    sScope_->__item = "";
    soap_default_wsdd__ProbeType(soap, req_);
    req_->Scopes = sScope_;
    req_->Types = ""; //"dn:NetworkVideoTransmitter";

    return soap ;
}
// Send Message, get Response from Devices who Support Onvif
int UserDiscoverDevices(struct soap *soap, wsdd__ProbeType *req, struct __wsdd__ProbeMatches *resp)
{
    int i = 0;
    int result = soap_send___wsdd__Probe(soap, MULTICAST_ADDRESS, NULL, req);
    int probeMatches_count = 0;
    while(result == SOAP_OK) {
        result = soap_recv___wsdd__ProbeMatches(soap, resp);
        if(result == SOAP_OK) {
            if(soap->error) {
                printf("[%d][%s][soap error 1: %d, %s, %s]\n",__LINE__, __func__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
                result = soap->error;
            } else {
                printf("[%d][%s][---- Onvif Device detected ----]\n", __LINE__, __func__);
                if (resp->wsdd__ProbeMatches != NULL) {
                    for(i = 0; i < resp->wsdd__ProbeMatches->__sizeProbeMatch; i++) {
                        printf("[%d][%s][__sizeProbeMatch: %d]\n", __LINE__, __func__, resp->wsdd__ProbeMatches->__sizeProbeMatch);
                        printf("[%d][%s][wsa__EndpointReference: %p]\n", __LINE__, __func__, resp->wsdd__ProbeMatches->ProbeMatch->wsa__EndpointReference);
                        printf("[%d][%s][Target EP Address: %s]\n", __LINE__, __func__, resp->wsdd__ProbeMatches->ProbeMatch->wsa__EndpointReference.Address);
                        printf("[%d][%s][Target Type: %s]\n", __LINE__, __func__, resp->wsdd__ProbeMatches->ProbeMatch->Types);
                        printf("[%d][%s][Target Service Address: %s]\n", __LINE__, __func__, resp->wsdd__ProbeMatches->ProbeMatch->XAddrs);
                        printf("[%d][%s][Target Metadata Version: %d]\n", __LINE__, __func__, resp->wsdd__ProbeMatches->ProbeMatch->MetadataVersion);
                        if(resp->wsdd__ProbeMatches->ProbeMatch->Scopes) {
                            printf("[%d][%s][Target Scopes Address:%s]\n", __LINE__, __func__, resp->wsdd__ProbeMatches->ProbeMatch->Scopes->__item);
                        }
                    }
                    printf("[%d][%s][NoMatche this time Count :%d]\n", __LINE__, __func__, ++probeMatches_count);
                }
                break;
            }
        }
        else if (soap->error) {
            printf("[%d][%s][soap error 2: %d, %s, %s]\n", __LINE__, __func__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
            result = soap->error;
        }
    }
    return result;
}

void UserGetCapabilities(struct soap *soap, struct __wsdd__ProbeMatches *resp,
                         struct _tds__GetCapabilities *capa_req, struct _tds__GetCapabilitiesResponse *capa_resp)
{
    int result = 0;
    capa_req->Category = (enum tt__CapabilityCategory *)soap_malloc(soap, sizeof(int));

    /// get capa_sequence count, 0 get nothing;
    capa_req->__sizeCategory = 1;
    // get MediaCapabilities /get others trans there;
    *(capa_req->Category) = (enum tt__CapabilityCategory)(tt__CapabilityCategory__Media);
    capa_resp->Capabilities = (struct tt__Capabilities*)soap_malloc(soap,sizeof(struct tt__Capabilities)) ;

    soap_wsse_add_UsernameTokenDigest(soap,"user", ONVIF_USER, ONVIF_PASSWORD);
    printf("[%d][%s][---- Gettting Device Capabilities ----]\n", __LINE__, __func__);
    result = soap_call___tds__GetCapabilities(soap, resp->wsdd__ProbeMatches->ProbeMatch->XAddrs, NULL, capa_req, capa_resp);
    printf("[%d][%s<%s!>]\n", __LINE__, __func__, (result == SOAP_OK) ? "成功":"失败");

    if (result != SOAP_OK) {
        printf("[%d][%s][Error Number:%d] [Falut Code:%s] [Falut Reason:%s]\n", __LINE__, __func__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
        return;
    }

    if(capa_resp->Capabilities != NULL) {
        printf("[%d][%s][MediaServerAddress:%s]\n", __LINE__, __func__, capa_resp->Capabilities->Media->XAddr);
    }

    return;
}

void UserGetSystemDataAndTime(struct soap *soap	,struct __wsdd__ProbeMatches *resp)
{
    int result = 0;
    struct _tds__GetSystemDateAndTime SystemDateAndTime;
    struct _tds__GetSystemDateAndTimeResponse SystemDateAndTimeResponse;

    soap_wsse_add_UsernameTokenDigest(soap,"user", ONVIF_USER, ONVIF_PASSWORD);
    printf("[%d][%s][---- Getting SystemD&T ----]\n", __LINE__, __func__);
    result = soap_call___tds__GetSystemDateAndTime(soap, resp->wsdd__ProbeMatches->ProbeMatch->XAddrs, NULL, &SystemDateAndTime, &SystemDateAndTimeResponse);
    printf("[%d][%s<%s!>][result = %d][soap_error = %d]\n", __LINE__, __func__, result ? "失败":"成功", result, soap->error);

    struct tt__DateTime *local = SystemDateAndTimeResponse.SystemDateAndTime->LocalDateTime;
    if(SystemDateAndTimeResponse.SystemDateAndTime != NULL) {
        printf("[%d][%s][DateTimeType:%d][Date:%d-%d-%d][Time:%d:%d:%d]\n", __LINE__, __func__
               , SystemDateAndTimeResponse.SystemDateAndTime->DateTimeType
               , local->Date->Year, local->Date->Month, local->Date->Day
               , local->Time->Hour, local->Time->Minute, local->Time->Second);
    } else {
        printf("[%d][%s][SystemDataError][Reason: Service Get Capabilities Error]\n", __LINE__, __func__);
    }
    return;
}

void UserGetProfiles(struct soap *soap,struct _trt__GetProfiles *trt__GetProfiles,
                     struct _trt__GetProfilesResponse *trt__GetProfilesResponse ,struct _tds__GetCapabilitiesResponse *capa_resp)
{
    int result=0 ;

    printf("[%d][%s][---- Getting Onvif Devices Profiles ----]\n", __LINE__, __func__);
    soap_wsse_add_UsernameTokenDigest(soap,"user", ONVIF_USER, ONVIF_PASSWORD);
    result = soap_call___trt__GetProfiles(soap, capa_resp->Capabilities->Media->XAddr, NULL, trt__GetProfiles, trt__GetProfilesResponse);
    printf("[%d][%s<%s!>]\n", __LINE__, __func__, (result == SOAP_EOF) ? "失败":"成功");
    ///NOTE: it may be regular if result isn't SOAP_OK.Because some attributes aren't supported by server.
    if (result == SOAP_EOF) {
        printf("[%d][%s][Error Number:%d] [Falut Code:%s] [Falut Reason:%s]\n", __LINE__, __func__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
        return;
    }
    if(trt__GetProfilesResponse->Profiles != NULL) {
        if(trt__GetProfilesResponse->Profiles->Name != NULL)
            printf("[%d][%s][Profiles Name:%s]\n",__LINE__, __func__, trt__GetProfilesResponse->Profiles->Name);
        if(trt__GetProfilesResponse->Profiles->VideoEncoderConfiguration != NULL)
            printf("[%d][%s][Profiles Token:%s]\n",__LINE__, __func__, trt__GetProfilesResponse->Profiles->VideoEncoderConfiguration->Name);
    }
    return;
}

// 获取提供给rtsp使用的流地址
void UserGetUri(struct soap *soap,struct _trt__GetStreamUri *trt__GetStreamUri,struct _trt__GetStreamUriResponse *trt__GetStreamUriResponse,
                struct _trt__GetProfilesResponse *trt__GetProfilesResponse,struct _tds__GetCapabilitiesResponse *capa_resp)
{
    int result=0 ;
    //Fill Request
    trt__GetStreamUri->StreamSetup = (struct tt__StreamSetup*)soap_malloc(soap,sizeof(struct tt__StreamSetup));//初始化，分配空间
    trt__GetStreamUri->StreamSetup->Stream = tt__StreamType__RTP_Unicast;

    trt__GetStreamUri->StreamSetup->Transport = (struct tt__Transport *)soap_malloc(soap, sizeof(struct tt__Transport));//初始化，分配空间
    trt__GetStreamUri->StreamSetup->Transport->Protocol = tt__TransportProtocol__RTSP;
    trt__GetStreamUri->StreamSetup->Transport->Tunnel = 0;
    trt__GetStreamUri->StreamSetup->__size = 1;
    trt__GetStreamUri->StreamSetup->__any = NULL;
    trt__GetStreamUri->StreamSetup->__anyAttribute =NULL;

    trt__GetStreamUri->ProfileToken = trt__GetProfilesResponse->Profiles->token ;

    printf("[%d][%s][---- Getting Onvif Devices StreamUri ----]\n", __LINE__, __func__);
    soap_wsse_add_UsernameTokenDigest(soap,"user", ONVIF_USER, ONVIF_PASSWORD);
    result = soap_call___trt__GetStreamUri(soap, capa_resp->Capabilities->Media->XAddr, NULL, trt__GetStreamUri, trt__GetStreamUriResponse);

    if (result != SOAP_OK) {
        printf("[%d][%s][Error Number:%d] [Falut Code:%s] [Falut Reason:%s]\n", __LINE__, __func__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
        return;
    }
    printf("[%d][%s][RTSP StreamURI: %s]\n",__LINE__, __func__, trt__GetStreamUriResponse->MediaUri->Uri);

    return;
}

// Set DeviceIpAddress, when Success ,dosome again;
int UserSetNetworkIPAddress(char *endpoint, struct soap *soap, struct _tds__SetNetworkInterfaces req, struct _tds__SetNetworkInterfacesResponse resp)
{
    if (endpoint == NULL || strlen(endpoint) == 0) {
        printf("[%d][%s][EndPoint is Null!]\n", __LINE__, __func__);
        return SOAP_ERR;
    }

    //Fill Request
    resp.RebootNeeded = xsd__boolean__true_;

    char interface[30] = "eth0";//"NetworkInterfaceToken_1";
    req.InterfaceToken = interface;
    struct tt__NetworkInterfaceSetConfiguration network;

    soap_wsse_add_UsernameTokenDigest(soap,"user", ONVIF_USER, ONVIF_PASSWORD);
    soap_default_tt__NetworkInterfaceSetConfiguration(soap, &network);

    enum xsd__boolean netEnable = xsd__boolean__true_;
    enum xsd__boolean ipv4Enable = xsd__boolean__true_;
    enum xsd__boolean DHCP = xsd__boolean__false_;

    network.Enabled = &netEnable;

    struct tt__IPv4NetworkInterfaceSetConfiguration tt_ipv4;
    soap_default_tt__IPv4NetworkInterfaceSetConfiguration(soap, &tt_ipv4);

    struct tt__PrefixedIPv4Address tt_preAddr;
    soap_default_tt__PrefixedIPv4Address(soap, &tt_preAddr);

    tt_preAddr.Address = ONVIF_DIST_NET_WORK_NEW ;//modify ipaddr
    tt_preAddr.PrefixLength = 24;
    tt_ipv4.Manual = &tt_preAddr;

    tt_ipv4.__sizeManual = 1;
    tt_ipv4.DHCP = &DHCP;
    tt_ipv4.Enabled = &ipv4Enable;
    network.IPv4 = &tt_ipv4;

    int mtuLen = 1499;
    network.MTU = &mtuLen;

    req.NetworkInterface = &network;

    printf("[%d][%s][---- Setting Device IPAddress:%s ----]\n", __LINE__, __func__, network.IPv4->Manual->Address);
    int result = soap_call___tds__SetNetworkInterfaces(soap,endpoint, NULL, &req, &resp);
    printf("[%d][%s<%s!>]\n", __LINE__, __func__, (result == SOAP_OK) ? "成功":"失败");

    if (result != SOAP_OK)
        printf("[%d][%s][Error Number:%d] [Falut Code:%s] [Falut Reason:%s]\n", __LINE__, __func__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));

    return result;
}

void doSome(struct soap *soap, struct __wsdd__ProbeMatches *resp)
{
    struct _tds__GetCapabilities capa_req;
    struct _tds__GetCapabilitiesResponse capa_resp;

    struct _trt__GetProfiles trt__GetProfiles;
    struct _trt__GetProfilesResponse trt__GetProfilesResponse;

    struct _trt__GetStreamUri trt__GetStreamUri;
    struct _trt__GetStreamUriResponse trt__GetStreamUriResponse;

    struct _tds__SetNetworkInterfaces setNetwork_req;
    struct _tds__SetNetworkInterfacesResponse setNetwork_resp;

    UserGetSystemDataAndTime(soap, resp);
    UserGetCapabilities(soap,resp, &capa_req, &capa_resp);
    UserGetProfiles(soap, &trt__GetProfiles, &trt__GetProfilesResponse, &capa_resp);
    UserGetUri(soap, &trt__GetStreamUri, &trt__GetStreamUriResponse, &trt__GetProfilesResponse, &capa_resp);

    UserSetNetworkIPAddress(capa_resp.Capabilities->Media->XAddr, soap, setNetwork_req, setNetwork_resp);
}

int main()
{
    printf("[%s][%d][%s][%s][---- ONVIF Device Test Started ----]\n", __FILE__, __LINE__, __TIME__, __func__);

    wsdd__ProbeType req;
    wsdd__ScopesType sScope;
    struct SOAP_ENV__Header header;

    struct soap *soap;
    soap = NewSoap(&header,soap,&req,&sScope);

    struct __wsdd__ProbeMatches resp;

    if (SOAP_OK == UserDiscoverDevices(soap, &req, &resp))
        doSome(soap, &resp);

    soap_destroy(soap);
    soap_end(soap);
    soap_free(soap);

    printf("[%s][%d][%s][%s][---- ONVIF Device Test Complete ----]\n", __FILE__, __LINE__, __TIME__, __func__);

    return 0;
}

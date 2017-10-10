#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "soapH.h"
#include "soapStub.h"
#include "wsdd.h"

#include <uuid/uuid.h>

#define ONVIF_USER          "admin"
#define ONVIF_PASSWORD      "bq111111"

// 通过capa 或者probeMatches 获取，此例不再重复
#define SERVICE_ENDPOINT    "http://192.168.0.66/onvif/device_service"

/*! 打开组播流程: Opt:Option  Req:Require  ReqN/1:N选一 R1o:first[Req]-else[Opt]
 *[R1o] GetMediaServiceAddress(获取访问节点,如其他回复capabilities或probematches成功也包含Media->Xaddress字段,可直接取用,不修改设备IP此节点不会发生修改)
 *[R1o]     |-->GetMediaProfile(获取配置文件,推荐修改配置参数时优先获取该配置所在的profile)
 *[Opt]         |-->Select profile with H.264 Video encoder configuration(启用选定的配置，有些设备提供多项配置，本例可以忽略，详情看文档)
 *[Opt]              |-->Setting video encoder configuration (如不需要修改配置,此步骤可忽略,直接使用getprofile中的profiletoken)
 *[Req2/1]                           |      |-->StartMulticastStreaming/StopMulticastStreaming(使用Onvif协议打开/关闭组播流)
 *[Req2/1]                           |-->GetStreamUri-->Rtsp......(获取组播流地址,相关流操作,交给Rtsp协议处理)
 */

struct soap* NewSoap(struct soap* soap)
{
    soap = soap_new();
    if(NULL == soap ) {
        printf("[%d][%s][Soap New Error]\n", __LINE__, __func__);
        return NULL;
    }

    soap->recv_timeout = 50;
    soap_set_namespaces(soap, namespaces);

    return soap ;
}
char* GetDeviceServices(struct soap* soap, struct _tds__GetServices *getServices, struct _tds__GetServicesResponse *getServicesResponse)
{
    char* mediaEndPoint = "";
    getServices->IncludeCapability = xsd__boolean__false_;

    printf("[%d][%s][---- Getting Device Services ----]\n", __LINE__, __func__);
    soap_wsse_add_UsernameTokenDigest(soap,"user", ONVIF_USER, ONVIF_PASSWORD);
    int result = soap_call___tds__GetServices(soap, SERVICE_ENDPOINT, NULL, getServices, getServicesResponse);
    printf("[%d][%s<%s!>][result = %d][soap_error = %d]\n", __LINE__, __func__, result ? "失败":"成功", result, soap->error);

    int i;
    if (getServicesResponse->Service != NULL) {
        for(i = 0; i < getServicesResponse->__sizeService; i++) {
            if (!strcmp(getServicesResponse->Service[i].Namespace, "http://www.onvif.org/ver10/media/wsdl")) {
                mediaEndPoint = getServicesResponse->Service[i].XAddr;
                printf("[%d][%s][MediaServiceAddress:%s]\n",__LINE__, __func__, mediaEndPoint);
                break;
            }
        }
    }
    return mediaEndPoint;
}
int GetProfiles(struct soap* soap, struct _trt__GetProfiles *trt__GetProfiles,
                  struct _trt__GetProfilesResponse *trt__GetProfilesResponse, char* media_ep)
{
    int result=0 ;
    printf("[%d][%s][---- Getting Profiles ----]\n", __LINE__, __func__);
    soap_wsse_add_UsernameTokenDigest(soap,"user", ONVIF_USER, ONVIF_PASSWORD);
    result = soap_call___trt__GetProfiles(soap, media_ep, NULL, trt__GetProfiles, trt__GetProfilesResponse);

    printf("[%d][%s<%s!>][result = %d][soap_error = %d]\n", __LINE__, __func__, result ? "失败":"成功", result, soap->error);
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
    return result;
}

int GetOpetions(struct soap* soap, struct _trt__GetProfilesResponse *profiles, char *media_ep)
{

    struct _trt__GetVideoEncoderConfigurationOptions *getConfigReq
            = (struct _trt__GetVideoEncoderConfigurationOptions*)soap_malloc(soap, sizeof(struct _trt__GetVideoEncoderConfigurationOptions));
    soap_default__trt__GetVideoEncoderConfigurationOptions(soap, getConfigReq);

    getConfigReq->ProfileToken = profiles->Profiles->token;
    getConfigReq->ConfigurationToken = profiles->Profiles->VideoEncoderConfiguration->token;

    struct _trt__GetVideoEncoderConfigurationOptionsResponse *getOptionsResponse
            = (struct _trt__GetVideoEncoderConfigurationOptionsResponse*)soap_malloc(soap, sizeof(struct _trt__GetVideoEncoderConfigurationOptionsResponse));

    soap_wsse_add_UsernameTokenDigest(soap,"user", ONVIF_USER, ONVIF_PASSWORD);

    printf("[%d][%s][---- Getting VideoEncoderConfigurationOptions ----]\n", __LINE__, __func__);
    int result = soap_call___trt__GetVideoEncoderConfigurationOptions(soap, media_ep, NULL, getConfigReq, getOptionsResponse);
    printf("[%d][%s<%s!>][result = %d][soap_error = %d]\n", __LINE__, __func__, result ? "失败":"成功", result, soap->error);

    if (result != SOAP_OK) {
        printf("[%d][%s][Error Number:%d] [Falut Code:%s] [Falut Reason:%s]\n", __LINE__, __func__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
        return result;
    }

    if (getOptionsResponse->Options->H264->FrameRateRange != NULL) {
        printf("[%d][%s][FrameRateRange:%d-%d]\n",__LINE__, __func__
               , getOptionsResponse->Options->H264->FrameRateRange->Min
               , getOptionsResponse->Options->H264->FrameRateRange->Max);
    }
    return result;
}

int SetConfig(struct soap* soap, struct _trt__GetProfilesResponse *profiles, char* media_ep)
{
    struct _trt__SetVideoEncoderConfiguration *setConfigReq
            = (struct _trt__SetVideoEncoderConfiguration *)soap_malloc(soap, sizeof(struct _trt__SetVideoEncoderConfiguration));
    soap_default__trt__SetVideoEncoderConfiguration(soap, setConfigReq);

    struct _trt__SetVideoEncoderConfigurationResponse *setConfigResponse
            = (struct _trt__SetVideoEncoderConfigurationResponse*)soap_malloc(soap, sizeof(struct _trt__SetVideoEncoderConfigurationResponse));

    struct tt__VideoEncoderConfiguration *mutConfig
            = (struct tt__VideoEncoderConfiguration*)soap_malloc(soap, sizeof(struct tt__VideoEncoderConfiguration));
    soap_default_tt__VideoEncoderConfiguration(soap, mutConfig);

    struct tt__MulticastConfiguration multicastconfig;
    multicastconfig.AutoStart = xsd__boolean__false_;
    multicastconfig.Port = 1234;
    struct tt__IPAddress address;
    address.Type = tt__IPType__IPv4;
    address.IPv4Address = "239.0.0.0";
    //不能 这样初始化 address.IPv6Address = NULL; soap_error12
    address.IPv6Address = "";
    multicastconfig.Address = &address;
    multicastconfig.__size = 1;
    multicastconfig.TTL = 1;

    mutConfig->Name = profiles->Profiles->VideoEncoderConfiguration->Name;
    mutConfig->UseCount = 1;
    mutConfig->Quality = 3;
    mutConfig->token = profiles->Profiles->VideoEncoderConfiguration->token;
    mutConfig->Encoding = tt__VideoEncoding__H264;
    struct tt__VideoResolution resolution;
    //HW混淆 产生soap_error2， 不提供此配置
    resolution.Height = 720;
    resolution.Width = 1280;
    mutConfig->Resolution = &resolution;
    mutConfig->Multicast = &multicastconfig;
    struct tt__VideoRateControl ratectrl;
    ratectrl.BitrateLimit = 16384;
    ratectrl.EncodingInterval = 1;
    ratectrl.FrameRateLimit = 12;
    mutConfig->RateControl = &ratectrl;
    mutConfig->SessionTimeout = 100;

    setConfigReq->Configuration = mutConfig;
    setConfigReq->ForcePersistence = xsd__boolean__false_;
    printf("[%d][%s][---- Setting Encoder Configuration ----]\n", __LINE__, __func__);
    soap_wsse_add_UsernameTokenDigest(soap, "user", ONVIF_USER, ONVIF_PASSWORD);
    int result = soap_call___trt__SetVideoEncoderConfiguration(soap, media_ep, NULL, setConfigReq, setConfigResponse);
    printf("[%d][%s<%s!>]\n", __LINE__, __func__, result ? "失败":"成功");
    if (result == SOAP_OK) {
        printf("[%d][%s][MulticastAddress:%s][port:%d]\n", __LINE__, __func__, address.IPv4Address, multicastconfig.Port);
        result = doStartMulticast(soap, profiles, media_ep);
    } else {
        printf("[%d][%s][Error Number:%d] [Falut Code:%s] [Falut Reason:%s]\n", __LINE__, __func__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
    }

    return result;
}

int doStartMulticast(struct soap* soap, struct _trt__GetProfilesResponse *profiles, char *media_ep)
{
    // 开启组播
    struct _trt__StartMulticastStreaming *_req
            = (struct _trt__StartMulticastStreaming*)soap_malloc(soap, sizeof(struct _trt__StartMulticastStreaming));
    soap_default__trt__StartMulticastStreaming(soap, _req);
    _req->ProfileToken = profiles->Profiles->token;
    struct _trt__StartMulticastStreamingResponse *_resp
            = (struct _trt__StartMulticastStreamingResponse*)soap_malloc(soap, sizeof(struct _trt__StartMulticastStreamingResponse));
    soap_default__trt__StartMulticastStreamingResponse(soap, _resp);

    printf("[%d][%s][---- Start Multicast Streaming ----]\n", __LINE__, __func__);
    soap_wsse_add_UsernameTokenDigest(soap,"user", ONVIF_USER, ONVIF_PASSWORD);
    int result = soap_call___trt__StartMulticastStreaming(soap, media_ep, NULL, _req, _resp);
    printf("[%d][%s<%s!>]\n", __LINE__, __func__, result ? "失败":"成功");

    if (result != SOAP_OK)
        printf("[%d][%s][Error Number:%d] [Falut Code:%s] [Falut Reason:%s]\n", __LINE__, __func__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
    return result;
}

void GetUri(struct soap *soap,struct _trt__GetStreamUri *trt__GetStreamUri,struct _trt__GetStreamUriResponse *trt__GetStreamUriResponse,
         struct _trt__GetProfilesResponse *profiles, char* media_ep)
{
    trt__GetStreamUri->StreamSetup = (struct tt__StreamSetup*)soap_malloc(soap,sizeof(struct tt__StreamSetup));//初始化，分配空间
    trt__GetStreamUri->StreamSetup->Stream = 1;

    trt__GetStreamUri->StreamSetup->Transport = (struct tt__Transport *)soap_malloc(soap, sizeof(struct tt__Transport));//初始化，分配空间
    trt__GetStreamUri->StreamSetup->Transport->Protocol = 0;
    trt__GetStreamUri->StreamSetup->Transport->Tunnel = 0;
    trt__GetStreamUri->StreamSetup->__size = 1;
    trt__GetStreamUri->StreamSetup->__any = NULL;
    trt__GetStreamUri->StreamSetup->__anyAttribute =NULL;

    trt__GetStreamUri->ProfileToken = profiles->Profiles->token;

    printf("[%d][%s][---- Getting Onvif Devices StreamUri ----]\n", __LINE__, __func__);

    soap_wsse_add_UsernameTokenDigest(soap,"user", ONVIF_USER, ONVIF_PASSWORD);
    int result = soap_call___trt__GetStreamUri(soap, media_ep, NULL, trt__GetStreamUri, trt__GetStreamUriResponse);

    if (result != SOAP_OK) {
        printf("[%d][%s][Error Number:%d] [Falut Code:%s] [Falut Reason:%s]\n", __LINE__, __func__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
        return;
    }
    if (trt__GetStreamUriResponse->MediaUri->Uri != NULL) {
        printf("[%d][%s][RTSP StreamURI: %s]\n",__LINE__, __func__, trt__GetStreamUriResponse->MediaUri->Uri);
    }
    return;
}


void FollowStepsToPerform(struct soap *soap)
{
    // 根据onvif test tool 中描述的开启组播步骤， 确保流程以及参数都是一样
    struct _trt__GetProfiles trt__GetProfiles;
    struct _trt__GetProfilesResponse profiles;

    struct _tds__GetServices getServices;
    soap_default__tds__GetServices(soap, &getServices);
    struct _tds__GetServicesResponse getServicesResponse;

    struct _trt__GetStreamUri trt__GetStreamUri;
    struct _trt__GetStreamUriResponse trt__GetStreamUriResponse;

    char *media_ep = GetDeviceServices(soap, &getServices, &getServicesResponse);
    if (media_ep != "") {
        GetProfiles(soap, &trt__GetProfiles, &profiles, media_ep);
        GetOpetions(soap, &profiles, media_ep);
        SetConfig(soap, &profiles, media_ep);

#if 1 // get multicast streamuri for rtsp
        GetUri(soap, &trt__GetStreamUri, &trt__GetStreamUriResponse, &profiles, media_ep);
#endif
    }
}

int main()
{
    printf("[%s][%d][%s][%s][---- ONVIF Device Test Started ----]\n", __FILE__, __LINE__, __TIME__, __func__);
    struct soap *soap;
    soap = NewSoap(soap);

    FollowStepsToPerform(soap);

    soap_destroy(soap);
    soap_end(soap);
    soap_free(soap);

    printf("[%s][%d][%s][%s][---- ONVIF Device Test Complete ----]\n", __FILE__, __LINE__, __TIME__, __func__);
    return 0;
}

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <uuid/uuid.h>

#include "soapH.h"
#include "soapStub.h"
#include "wsdd.h"

#define ONVIF_USER          "admin"
#define ONVIF_PASSWORD      "bq111111"

#define MEDIA_ENDPOINT      "http://www.onvif.org/ver10/media/wsdl"
#define IMAGE_ENDPOINT      "http://www.onvif.org/ver20/imaging/wsdl"

// 通过capa 或者probeMatches 获取，此例不再重复
#define SERVICE_ENDPOINT    "http://192.168.0.236/onvif/device_service"

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
char* GetDeviceServices(struct soap* soap, struct _tds__GetServices *getServices, struct _tds__GetServicesResponse *getServicesResponse, const char* specname)
{
    char* endPoint = "";
    getServices->IncludeCapability = xsd__boolean__false_;

    printf("[%d][%s][---- Getting Device Services ----]\n", __LINE__, __func__);
    soap_wsse_add_UsernameTokenDigest(soap,"user", ONVIF_USER, ONVIF_PASSWORD);
    int result = soap_call___tds__GetServices(soap, SERVICE_ENDPOINT, NULL, getServices, getServicesResponse);
    printf("[%d][%s<%s!>][result = %d][soap_error = %d]\n", __LINE__, __func__, result ? "失败":"成功", result, soap->error);

    int i;
    if (getServicesResponse->Service != NULL) {
        for(i = 0; i < getServicesResponse->__sizeService; i++) {
            if (!strcmp(getServicesResponse->Service[i].Namespace, specname)) {
                endPoint = getServicesResponse->Service[i].XAddr;
                printf("[%d][%s][MediaServiceAddress:%s]\n",__LINE__, __func__, endPoint);
                break;
            }
        }
    }
    return endPoint;
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

#if 1
    struct tt__MulticastConfiguration multicastconfig;
    multicastconfig.AutoStart = xsd__boolean__false_;
    multicastconfig.Port = 10001;
    struct tt__IPAddress address;
    address.Type = tt__IPType__IPv4;
    address.IPv4Address = "232.0.0.1";
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
    ratectrl.BitrateLimit = 8000;
    ratectrl.EncodingInterval = 1;
    ratectrl.FrameRateLimit = 24;
    mutConfig->RateControl = &ratectrl;
    mutConfig->SessionTimeout = 100;
#endif

    setConfigReq->Configuration = mutConfig;
    setConfigReq->ForcePersistence = xsd__boolean__false_;
    printf("[%d][%s][---- Setting Encoder Configuration ----]\n", __LINE__, __func__);
    soap_wsse_add_UsernameTokenDigest(soap, "user", ONVIF_USER, ONVIF_PASSWORD);
    int result = soap_call___trt__SetVideoEncoderConfiguration(soap, media_ep, NULL, setConfigReq, setConfigResponse);
    return result;
}
#if 0
int getVideoSource(struct soap* soap, char* media_ep,struct _trt__GetVideoSources *getVideoSources, struct _trt__GetVideoSourcesResponse *getVideosourcesResp)
{
    int result = soap_call___trt__GetVideoSources(soap, media_ep, NULL, getVideoSources, getVideosourcesResp);
    return result;
}
#endif
int getImageSetting(struct soap* soap, struct _trt__GetVideoSourcesResponse *getVideosourcesResp, struct _trt__GetProfilesResponse *profiles, char* image_ep)
{
    struct _timg__GetImagingSettings *getImageReq
            = (struct _timg__GetImagingSettings *)soap_malloc(soap, sizeof(struct _timg__GetImagingSettings));
    soap_default__timg__GetImagingSettings(soap, getImageReq);

    struct _timg__GetImagingSettingsResponse *getImageResp
            = (struct _timg__GetImagingSettingsResponse *)soap_malloc(soap, sizeof(struct _timg__GetImagingSettingsResponse));
    soap_default__timg__GetImagingSettingsResponse(soap, getImageResp);

    printf("[%d][%s][---- Getting ImageSetting ----]\n", __LINE__, __func__);
    getImageReq->VideoSourceToken = profiles->Profiles->VideoSourceConfiguration->SourceToken;

    soap_wsse_add_UsernameTokenDigest(soap,"user", ONVIF_USER, ONVIF_PASSWORD);
    int result = soap_call___timg__GetImagingSettings(soap, image_ep, NULL, getImageReq, getImageResp);

    printf("[%d][%s][---- Getting ImageSetting down ----][result = %d]\n", __LINE__, __func__, result);
    if (result == SOAP_OK) {
    printf("[brightness:%f]\n[contrast:%f]\n[colorsaturation:%f]\n", *getImageResp->ImagingSettings->Brightness
           , *getImageResp->ImagingSettings->Contrast, *getImageResp->ImagingSettings->ColorSaturation);
    }
    return result;
}

int setImageSetting(struct soap* soap, struct _trt__GetVideoSourcesResponse *getVideosourcesResp, struct _trt__GetProfilesResponse *profiles, char* image_ep)
{
    struct _timg__SetImagingSettings *setImageReq
            = (struct _timg__SetImagingSettings *)soap_malloc(soap, sizeof(struct _timg__SetImagingSettings));
    soap_default__timg__SetImagingSettings(soap, setImageReq);

    struct _timg__SetImagingSettingsResponse *setImageResp
            = (struct _timg__SetImagingSettingsResponse *)soap_malloc(soap, sizeof(struct _timg__SetImagingSettingsResponse));
    soap_default__timg__SetImagingSettingsResponse(soap, setImageResp);

    struct tt__ImagingSettings20 *imagesetting
            = (struct tt__ImagingSettings20*)soap_malloc(soap, sizeof(struct tt__ImagingSettings20));

    setImageReq->VideoSourceToken = profiles->Profiles->VideoSourceConfiguration->SourceToken;
    setImageReq->ForcePersistence = xsd__boolean__false_;
    float val = 33.33;
    imagesetting->Brightness = &val;
    imagesetting->ColorSaturation = &val;
    imagesetting->Contrast = &val;

    setImageReq->ImagingSettings = imagesetting;

    printf("[%d][%s][---- Setting image setting ----]\n", __LINE__, __func__);
    soap_wsse_add_UsernameTokenDigest(soap,"user", ONVIF_USER, ONVIF_PASSWORD);
    int result = soap_call___timg__SetImagingSettings(soap, image_ep, NULL, setImageReq, setImageResp);
    printf("[%d][%s][---- Setting image setting ----][result = %d]\n", __LINE__, __func__, result);
    return result;
}

void FollowStepsToPerform(struct soap *soap)
{
    // 根据onvif test tool 中描述的开启组播步骤， 确保流程以及参数都是一样
    struct _trt__GetProfiles trt__GetProfiles;
    struct _trt__GetProfilesResponse profiles;

    struct _tds__GetServices getServices;
    soap_default__tds__GetServices(soap, &getServices);
    struct _tds__GetServicesResponse getServicesResponse;

    struct _trt__GetVideoSources getVideoSources;
    struct _trt__GetVideoSourcesResponse getVideoSourcesResp;

    char *media_ep = GetDeviceServices(soap, &getServices, &getServicesResponse, MEDIA_ENDPOINT);
    if (media_ep != "") {
        GetProfiles(soap, &trt__GetProfiles, &profiles, media_ep);
        GetOpetions(soap, &profiles, media_ep);
        SetConfig(soap, &profiles, media_ep);
    //    getVideoSource(soap, media_ep, getVideoSources, getVideoSourcesResp);
        char *image_ep = GetDeviceServices(soap, &getServices, &getServicesResponse, IMAGE_ENDPOINT);
        if (image_ep != "") {
            getImageSetting(soap, &getVideoSourcesResp, &profiles, image_ep);
            setImageSetting(soap, &getVideoSourcesResp, &profiles, image_ep);
            getImageSetting(soap, &getVideoSourcesResp, &profiles, image_ep);
        }
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

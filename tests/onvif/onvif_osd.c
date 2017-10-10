
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "soapH.h"
#include "soapStub.h"
#include "wsdd.h"

#include <uuid/uuid.h>

#define ONVIF_USER          "admin"
#define ONVIF_PASSWORD      "bq111111"

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
            if (!strcmp(getServicesResponse->Service[i].Namespace, SOAP_NAMESPACE_OF_trt)) {
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
    int result = 0;
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

void SetOsd(struct soap*soap, char*ep, struct _trt__SetOSD *req, struct _trt__SetOSDResponse *resp, struct _trt__GetOSDsResponse *osdcfg)
{
    struct tt__OSDConfiguration osd;
    osd = *(osdcfg->OSDs);
    char * text = "onvif_test_osd";
    osd.TextString->PlainText = text;
    req->OSD = &osd;
    soap_wsse_add_UsernameTokenDigest(soap, "user", ONVIF_USER, ONVIF_PASSWORD);
    soap_call___trt__SetOSD(soap, ep, NULL, req, resp);
}

void getOsds(struct soap* soap , char* ep, struct _trt__GetProfilesResponse *trt__GetProfilesResponse, struct _trt__GetOSDs *trt__GetOSDs, struct _trt__GetOSDsResponse *trt__GetOSDsResponse)
{
    trt__GetOSDs->ConfigurationToken = trt__GetProfilesResponse->Profiles->VideoSourceConfiguration->token;

    soap_wsse_add_UsernameTokenDigest(soap, "user", ONVIF_USER, ONVIF_PASSWORD);
    soap_call___trt__GetOSDs(soap, ep, NULL, trt__GetOSDs, trt__GetOSDsResponse);
}

void FollowStepsToPerform(struct soap *soap)
{
    struct _trt__GetProfiles trt__GetProfiles;
    struct _trt__GetProfilesResponse profiles;

    struct _tds__GetServices getServices;
    soap_default__tds__GetServices(soap, &getServices);
    struct _tds__GetServicesResponse getServicesResponse;

    struct _trt__GetOSDs trt__GetOSDs;
    struct _trt__GetOSDsResponse trt__GetOSDsResponse;

    struct _trt__SetOSD trt__SetOSD;
    struct _trt__SetOSDResponse trt__SetOSDResponse;

    char *media_ep = GetDeviceServices(soap, &getServices, &getServicesResponse);
    if (media_ep != "") {
        GetProfiles(soap, &trt__GetProfiles, &profiles, media_ep);
        getOsds(soap, media_ep, &profiles, &trt__GetOSDs, &trt__GetOSDsResponse);
        SetOsd(soap, media_ep, &trt__SetOSD, &trt__SetOSDResponse, &trt__GetOSDsResponse);
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

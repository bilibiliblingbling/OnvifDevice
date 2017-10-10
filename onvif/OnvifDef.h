#ifndef __ONVIF_DEVICE_PLUGIN_ONVIF_ONVIFDEF_H__
#define __ONVIF_DEVICE_PLUGIN_ONVIF_ONVIFDEF_H__

#include "soap/soapH.h"
#include "soap/soapStub.h"
#include "soap/gsoap/stdsoap2.h"

#include <swift/smart_ptr.hpp>
#include <swift/assert.hpp>

/// WS-authon
#define SIGNATURE_ROLE "user"

/// Discovery MultiAdress
#define PROBE_ADDRESS_HEADER "soap.udp://"
#define PROBE_ADDRESS_PORT ":3702"

#define PROBE_MULTICAST_ADDRESS "soap.udp://239.255.255.250:3702"

/// Rename soap
typedef struct soap OnvifSoap;
/// Rename soap status
typedef soap_status OnvifSoapResult;
/// Rename Request Response
// Servers 请求回复
typedef struct _tds__GetServices                ServicesReq;
typedef struct _tds__GetServicesResponse        ServicesResp;

/// Capabilities
typedef struct _tds__GetCapabilities            CapabilitiesReq;
typedef struct _tds__GetCapabilitiesResponse    CapabilitiesResp;

/// Discovery
typedef wsdd__ProbeType                         ProbeReq;
typedef struct __wsdd__ProbeMatches             ProbeResp;

/// Media-ver10
typedef struct _trt__GetProfiles                ProfilesReq;
typedef struct _trt__GetProfilesResponse        ProfilesResp;

// Media_stream
typedef struct _trt__GetStreamUri               StreamUriReq;
typedef struct _trt__GetStreamUriResponse       StreamUriResp;
typedef struct _trt__StartMulticastStreaming    StartMulticastReq;
typedef struct _trt__StartMulticastStreamingResponse StartMulticastResp;
typedef struct _trt__StopMulticastStreaming     StopMulticastReq;
typedef struct _trt__StopMulticastStreamingResponse StopMulticastResp;

// Media_videosetting
typedef struct _trt__GetVideoEncoderConfigurationOptions VideoEncoderConfigOptReq;
typedef struct _trt__GetVideoEncoderConfigurationOptionsResponse VideoEncoderConfigOptResp;
typedef struct _trt__SetVideoEncoderConfiguration SetVideoEncoderConfigReq;
typedef struct _trt__SetVideoEncoderConfigurationResponse SetVideoEncoderConfigResp;

typedef struct tt__Profile                      MediaProfiles;
typedef struct tt__VideoEncoderConfiguration    MediaConfig;
typedef struct tt__VideoResolution              MediaResolution;
typedef struct tt__VideoRateControl             MediaRateControl;
typedef struct tt__OSDConfiguration             OSDConfig;

// Media_OSD
#define OSD_TYPE_DATATIME "DateAndTime"
#define OSD_TYPE_PLAINTEXT "Plain"
typedef struct _trt__GetOSDs                    GetOSDsConfig;
typedef struct _trt__GetOSDsResponse            GetOSDsConfigResp;

typedef struct _trt__SetOSD                     SetOSDConfig;
typedef struct _trt__SetOSDResponse             SetOSDConfigResp;
typedef struct _trt__CreateOSD                  CreateOSDConfig;
typedef struct _trt__CreateOSDResponse          CreateOSDConfigResp;
typedef struct _trt__DeleteOSD                  DeleteOSDConfig;
typedef struct _trt__DeleteOSDResponse          DeleteOSDConfigResp;

typedef struct _trt__AddVideoSourceConfiguration AddVideoSource;
typedef struct _trt__AddVideoSourceConfigurationResponse AddVideoSourceResp;
/// PTZ-ver20
#define PTZ_MOVE_SPEED 0.2
typedef struct tt__PTZSpeed                     PTZArg;
typedef struct _tptz__ContinuousMove            PTZMoveReq;
typedef struct _tptz__ContinuousMoveResponse    PTZMoveResp;
typedef struct _tptz__GotoHomePosition          GotoHomePos;
typedef struct _tptz__GotoHomePositionResponse  GotoHomePosResp;

typedef struct tt__Vector2D                     PTZPantilt;
typedef struct tt__Vector1D                     PTZZoom;
/// Image-ver20
// Image_setting
typedef struct tt__ImagingSettings20            ImageConfig;
typedef struct _timg__SetImagingSettings        SetImagingReq;
typedef struct _timg__SetImagingSettingsResponse SetImagingResp;
typedef struct _timg__GetImagingSettings        GetImagingReq;
typedef struct _timg__GetImagingSettingsResponse GetImagingResp;

#endif

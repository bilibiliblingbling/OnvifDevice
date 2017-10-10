#include "OnvifMedia.h"
#include "soap/gsoap/plugin/wsseapi.h"

OnvifMedia::OnvifMedia(const string &mediaServername, SoapAuthentication auth, bool bIsConnect)
    : OnvifBasic(auth, bIsConnect)
    , m_mediaServerAddr(mediaServername)
{
    InitReqResp();
    if (GetProfiles() == SOAP_OK) {
        // if (CreateSubStreamMediaSourceConfigure() != SOAP_OK) {}

    }
}

OnvifMedia::~OnvifMedia()
{
}

const string OnvifMedia::GetStreamURI(int streamID)
{
    m_getStreamUriReq->StreamSetup->Stream = tt__StreamType__RTP_Multicast; //tt__StreamType__RTP_Unicast;

    m_getStreamUriReq->StreamSetup->Transport->Protocol = tt__TransportProtocol__RTSP;//tt__TransportProtocol__UDP;
    m_getStreamUriReq->StreamSetup->Transport->Tunnel = 0;
    m_getStreamUriReq->StreamSetup->__size = 1;
    m_getStreamUriReq->StreamSetup->__any = NULL;
    m_getStreamUriReq->StreamSetup->__anyAttribute =NULL;

    if (streamID == 1) {
        m_getStreamUriReq->ProfileToken = m_getProfilesResp->Profiles->token;
    } else if (streamID > 64) {
        m_getStreamUriReq->ProfileToken = m_getProfilesResp->Profiles[1].token;
    }

    TokenDigest();
    OnvifSoapResult result = soap_call___trt__GetStreamUri(m_soap, m_mediaServerAddr.c_str(), NULL, m_getStreamUriReq, m_getStreamUriResp);
    if (result == SOAP_OK)
        return m_getStreamUriResp->MediaUri->Uri;

    return NULL;
}

OnvifSoapResult OnvifMedia::StartMulticast(bool bIsMainStream)
{
    if (bIsMainStream) {
        m_startMulticastReq->ProfileToken = m_getProfilesResp->Profiles->token;
    } else {
        m_startMulticastReq->ProfileToken = m_getProfilesResp->Profiles[1].token;
    }
    TokenDigest();
    OnvifSoapResult result = soap_call___trt__StartMulticastStreaming(m_soap,m_mediaServerAddr.c_str(), NULL, m_startMulticastReq, m_startMulticastResp);
    return result; //soap_call___trt__StartMulticastStreaming(m_soap,m_mediaServerAddr.c_str(), NULL, m_startMulticastReq, m_startMulticastResp);
}

OnvifSoapResult OnvifMedia::StopMulticast(bool bIsMainStream)
{
    if (bIsMainStream) {
        m_stopMulticastReq->ProfileToken = m_getProfilesResp->Profiles->token;
    } else {
        m_stopMulticastReq->ProfileToken = m_getProfilesResp->Profiles[1].token;
    }
    TokenDigest();
    return soap_call___trt__StopMulticastStreaming(m_soap, m_mediaServerAddr.c_str(), NULL, m_stopMulticastReq, m_stopMulticastResp);
}

OnvifSoapResult OnvifMedia::GetProfiles()
{
    TokenDigest();
    OnvifSoapResult result =  soap_call___trt__GetProfiles(m_soap, m_mediaServerAddr.c_str(), NULL, m_getProfilesReq, m_getProfilesResp);
    return result; //soap_call___trt__GetProfiles(m_soap, m_mediaServerAddr.c_str(), NULL, m_getProfilesReq, m_getProfilesResp);
}

ProfilesResp *OnvifMedia::GetMediaProfiles()
{
    if (GetProfiles() != SOAP_OK)
        return NULL;

    return m_getProfilesResp;
}

OnvifSoapResult OnvifMedia::SetVideoEncoderConfig(OnvifStreamArguments &configArgs, bool bIsMainStream)
{
    GetProfiles();
    struct tt__VideoEncoderConfiguration *encoderConfig
            = (struct tt__VideoEncoderConfiguration*)soap_malloc(m_soap, sizeof(struct tt__VideoEncoderConfiguration));
    soap_default_tt__VideoEncoderConfiguration(m_soap, encoderConfig);
    struct tt__VideoRateControl ratectrl;
    if (bIsMainStream) {
        encoderConfig = m_getProfilesResp->Profiles->VideoEncoderConfiguration;
        ratectrl.EncodingInterval = m_getProfilesResp->Profiles->VideoEncoderConfiguration->RateControl->EncodingInterval;
    } else {
        encoderConfig = m_getProfilesResp->Profiles[1].VideoEncoderConfiguration;
        ratectrl.EncodingInterval = m_getProfilesResp->Profiles[1].VideoEncoderConfiguration->RateControl->EncodingInterval;
    }

    m_setConfigOptReq->ForcePersistence = xsd__boolean__false_;

    struct tt__VideoResolution resolution;
    resolution.Height = configArgs.mediaArg.resolution.height;
    resolution.Width = configArgs.mediaArg.resolution.width;
    encoderConfig->Resolution = &resolution;
    ratectrl.BitrateLimit = configArgs.mediaArg.bitrate;
    ratectrl.FrameRateLimit = configArgs.mediaArg.framerate;
    encoderConfig->RateControl = &ratectrl;

    m_setConfigOptReq->Configuration = encoderConfig;
    TokenDigest();
    OnvifSoapResult result = soap_call___trt__SetVideoEncoderConfiguration(m_soap, m_mediaServerAddr.c_str(), NULL, m_setConfigOptReq, m_setConfigOptResp);
    return result;
}

OnvifSoapResult OnvifMedia::SetMulticastEncoderConfig(unsigned long multAddr, unsigned short destPort, bool bIsMainStream)
{
    struct tt__VideoEncoderConfiguration *mutConfig
            = (struct tt__VideoEncoderConfiguration*)soap_malloc(m_soap, sizeof(struct tt__VideoEncoderConfiguration));
    soap_default_tt__VideoEncoderConfiguration(m_soap, mutConfig);

    struct tt__VideoRateControl ratectrl;
    struct tt__MulticastConfiguration multicastconfig;
    unsigned long multAddrNet = htonl(multAddr);
    char* strMultAddr = ::inet_ntoa((in_addr&)multAddrNet);

    multicastconfig.AutoStart = xsd__boolean__false_;
    multicastconfig.Port = (int)destPort;
    struct tt__IPAddress address;
    address.Type = tt__IPType__IPv4;
    address.IPv4Address = strMultAddr;

    address.IPv6Address = NULL;
    multicastconfig.Address = &address;
    multicastconfig.__size = 2;
    multicastconfig.__any = NULL;
    multicastconfig.TTL = 1;

    if (bIsMainStream) {
        struct tt__VideoEncoderConfiguration *mainEncode = m_getProfilesResp->Profiles->VideoEncoderConfiguration;
        mutConfig->Name = mainEncode->Name;
        mutConfig->UseCount = mainEncode->UseCount;
        mutConfig->Quality = mainEncode->Quality;
        mutConfig->token = mainEncode->token;
        mutConfig->Encoding = tt__VideoEncoding__H264;
        mutConfig->__any = mainEncode->__any;
        mutConfig->__size = mainEncode->__size;

        mutConfig->Resolution = mainEncode->Resolution;
        ratectrl.BitrateLimit = mainEncode->RateControl->BitrateLimit;
        ratectrl.EncodingInterval = mainEncode->RateControl->EncodingInterval;
        ratectrl.FrameRateLimit = mainEncode->RateControl->FrameRateLimit;
        mutConfig->RateControl = mainEncode->RateControl;
        mutConfig->SessionTimeout = mainEncode->SessionTimeout;
    } else {
        struct tt__VideoEncoderConfiguration *subEncode = m_getProfilesResp->Profiles[1].VideoEncoderConfiguration;
        mutConfig->Name = subEncode->Name;
        mutConfig->UseCount = subEncode->UseCount;
        mutConfig->Quality = subEncode->Quality;
        mutConfig->token = subEncode->token;
        mutConfig->Encoding = tt__VideoEncoding__H264;
        mutConfig->__any = subEncode->__any;
        mutConfig->__size = subEncode->__size;

        mutConfig->Resolution = subEncode->Resolution;
        mutConfig->RateControl = subEncode->RateControl;
        mutConfig->SessionTimeout = subEncode->SessionTimeout;
    }
    mutConfig->Multicast = &multicastconfig;
    m_setConfigOptReq->Configuration = mutConfig;
    m_setConfigOptReq->ForcePersistence = xsd__boolean__false_;

    TokenDigest();
    OnvifSoapResult result = soap_call___trt__SetVideoEncoderConfiguration(m_soap, m_mediaServerAddr.c_str(), NULL, m_setConfigOptReq, m_setConfigOptResp);

    return result;
}

GetOSDsConfigResp *OnvifMedia::GetOSDs(bool bIsMainStream)
{
    OnvifSoapResult result = GetProfiles();
    if (result == SOAP_OK) {
        if (bIsMainStream) {
            m_getOSDsReq->ConfigurationToken = m_getProfilesResp->Profiles->VideoSourceConfiguration->token;
        } else {
            m_getOSDsReq->ConfigurationToken = m_getProfilesResp->Profiles[1].VideoSourceConfiguration->token;
        }
        TokenDigest();
        result = soap_call___trt__GetOSDs(m_soap, m_mediaServerAddr.c_str(), NULL, m_getOSDsReq, m_getOSDsResp);
        if (result == SOAP_OK) {
            return m_getOSDsResp;
        }
    }
    return NULL;
}

OnvifSoapResult OnvifMedia::SetOSD(OnvifOSDArguments *configArgs, bool bIsMainStream, bool bIsDateTimeSetting)
{
    struct tt__OSDConfiguration osd;
    GetOSDs(bIsMainStream);
    osd.Type = m_getOSDsResp->OSDs->Type;
    osd.VideoSourceConfigurationToken = m_getOSDsResp->OSDs->VideoSourceConfigurationToken;

    if (bIsDateTimeSetting) {
        for(int i = 0; i < m_getOSDsResp->__sizeOSDs; i++) {
           if (!strcmp(m_getOSDsResp->OSDs[i].token, configArgs->datetimeToken)){
                osd = m_getOSDsResp->OSDs[i];
                osd.Position->Pos->x = configArgs->datetimePos->x;
                osd.Position->Pos->y = configArgs->datetimePos->y;
            }
        }
    } else {
        for(int i = 0; i < m_getOSDsResp->__sizeOSDs; i++) {
            if (!strcmp(m_getOSDsResp->OSDs[i].token, configArgs->customToken)){
                osd = m_getOSDsResp->OSDs[i];
                osd.Position->Pos->x = configArgs->customPos->x;
                osd.Position->Pos->y = configArgs->customPos->y;
                osd.TextString->PlainText = configArgs->customText;
            }
        }
    }

    m_setOSDReq->OSD = &osd;
    TokenDigest();
    OnvifSoapResult result = soap_call___trt__SetOSD(m_soap, m_mediaServerAddr.c_str(), NULL, m_setOSDReq, m_setOSDResp);
    return result;
}

OnvifSoapResult OnvifMedia::CreateOSD(OnvifOSDArguments *configArgs, bool bIsMainStream, bool bIsDateTimeSetting)
{
    struct tt__OSDConfiguration osd;
    struct tt__OSDReference reftoken;
    if (bIsMainStream) {
        reftoken.__item = m_getProfilesResp->Profiles->VideoSourceConfiguration->token;
    } else {
        reftoken.__item = m_getProfilesResp->Profiles[1].VideoSourceConfiguration->token;
    }
    //reftoken.__item = "VideoSourceToken";
    reftoken.__anyAttribute = NULL;
    //char* token = "";
    osd.token = NULL;
    osd.VideoSourceConfigurationToken = &reftoken;
    osd.Type = tt__OSDType__Text;
    osd.__anyAttribute = NULL;
    // POS
    struct tt__OSDPosConfiguration pos;
    struct tt__Vector coor;
    struct tt__OSDPosConfigurationExtension posextension;
    char *postype = "Custom";
    posextension.__any = NULL;
    posextension.__size = 0;
    posextension.__anyAttribute = NULL;

    // PlainText
    struct tt__OSDTextConfiguration textstr;
    struct tt__Color color;
    struct tt__OSDColor fontcolor;
    struct tt__OSDColor backgroundcolor;
    struct tt__OSDTextConfigurationExtension textstrExtension;
    char *strtype;
    char *datefmt;
    char *timefmt;

    int fontsize = 64;
    int *FontSize = &fontsize;
    textstr.__anyAttribute = NULL;
    color.X = 16;
    color.Y = 128;
    color.Z = 128;
    color.Colorspace = "http://www.onvif.org/ver10/colorspace/YCbCr";
    int transparent = 0;

    fontcolor.Color = &color;
    fontcolor.Transparent = &transparent;
    fontcolor.__anyAttribute = NULL;
    textstr.FontColor = &fontcolor;

    backgroundcolor.Color = &color;
    backgroundcolor.Transparent = &transparent;
    backgroundcolor.__anyAttribute = NULL;
    textstr.BackgroundColor = &backgroundcolor;

    textstrExtension.__any = NULL;
    textstrExtension.__size = 0;
    textstrExtension.__anyAttribute = NULL;
    textstr.Extension = &textstrExtension;

    //OSD_Extension
    struct tt__OSDConfigurationExtension extension;
    extension.__any = NULL;
    extension.__size = 0;
    extension.__anyAttribute = NULL;

    if (bIsDateTimeSetting) {
        coor.x = configArgs->datetimePos->x;
        coor.y = configArgs->datetimePos->y;
        strtype = "DateAndTime";
        datefmt = "MM/dd/yyyy";
        timefmt = "HH:mm:ss";
        textstr.Type = strtype;
        textstr.PlainText = NULL;
        textstr.FontSize = FontSize;
        textstr.DateFormat = datefmt;
        textstr.TimeFormat = timefmt;
    } else {
        coor.x = configArgs->customPos->x;
        coor.y = configArgs->customPos->y;
        strtype = "Plain";
        textstr.Type = strtype;
        textstr.FontSize = FontSize;
        textstr.PlainText = configArgs->customText;
        textstr.TimeFormat = NULL;
        textstr.DateFormat = NULL;
    }
    pos.Pos = &coor;
    pos.Type = postype;
    pos.__anyAttribute = NULL;
    pos.Extension = &posextension;

    osd.Position = &pos;
    osd.TextString = &textstr;
    osd.Image = NULL;
    osd.Extension = &extension;
    m_createOSDReq->OSD = &osd;
    TokenDigest();
    OnvifSoapResult result = soap_call___trt__CreateOSD(m_soap, m_mediaServerAddr.c_str(), NULL, m_createOSDReq, m_createOSDResp);
    if (result == SOAP_OK) {
        if (bIsDateTimeSetting) {
            configArgs->datetimeToken = m_createOSDResp->OSDToken;
            configArgs->bShowDateTime = true;
        } else {
            configArgs->customToken = m_createOSDResp->OSDToken;
            configArgs->bShowCustomText = true;
        }
    }
    return result;
}

OnvifSoapResult OnvifMedia::DeleteOSD(OnvifOSDArguments *configArgs, bool bIsMainStream, bool bIsDateTimeSetting)
{
    char *token;
    if (bIsDateTimeSetting) {
        token = configArgs->datetimeToken;
    } else {
        token = configArgs->customToken;
    }

    OnvifSoapResult result = SOAP_EOF;
    if (token != NULL) {
        m_deleteOSDReq->OSDToken = token;
        TokenDigest();
        result = soap_call___trt__DeleteOSD(m_soap, m_mediaServerAddr.c_str(), NULL, m_deleteOSDReq, m_deleteOSDResp);
    }

    if (result == SOAP_OK) {
        if (bIsDateTimeSetting) {
            configArgs->ResetDateTimeOSD();
        } else {
            configArgs->ResetCustomTextOSD();
        }
    }
    return result;
}

OnvifSoapResult OnvifMedia::CreateSubStreamMediaSourceConfigure()
{
    /** 如果为辅码流创建mediasource失败，设备无法未辅码流提供单独的OSD等其他功能
        测试用hik设备多通道共用1个mediasource　所以不支持主辅流不同字幕，
        功能已经实现，目前实际上主辅流设置分别是profile1_1/profile_2的mediasource
        调试时如发现resp　字段中"__sizeVideoSourceTokenAvailable" > 1时
        或发现optresp中,__"sizeVideoSourceTokensAvailable" > 1 时
        还是不支持主辅流分别配置osd,可以尝试开启此接口,
        将正确的mediasourcetoken(与主码流不同)填入addvideosourcereq中
      */

    _trt__GetVideoSourceConfigurations *req = (_trt__GetVideoSourceConfigurations*)soap_malloc(m_soap, sizeof(_trt__GetVideoSourceConfigurations));
    _trt__GetVideoSourceConfigurationsResponse *resp = (_trt__GetVideoSourceConfigurationsResponse*)soap_malloc(m_soap, sizeof(_trt__GetVideoSourceConfigurationsResponse));

    TokenDigest();
    int res = soap_call___trt__GetVideoSourceConfigurations(m_soap, m_mediaServerAddr.c_str(),NULL, req, resp);

    _trt__GetVideoSourceConfigurationOptions *optreq = (_trt__GetVideoSourceConfigurationOptions*)soap_malloc(m_soap, sizeof(_trt__GetVideoSourceConfigurationOptions));
    _trt__GetVideoSourceConfigurationOptionsResponse *optresp = (_trt__GetVideoSourceConfigurationOptionsResponse*)soap_malloc(m_soap, sizeof(_trt__GetVideoSourceConfigurationOptionsResponse));

    optreq->ProfileToken = m_getProfilesResp->Profiles[1].token;
    optreq->ConfigurationToken = resp->Configurations->token;
    TokenDigest();
    int optres = soap_call___trt__GetVideoSourceConfigurationOptions(m_soap, m_mediaServerAddr.c_str(), NULL,optreq, optresp);

    m_addVideoSourceReq->ConfigurationToken = resp->Configurations[1].token;
    m_addVideoSourceReq->ProfileToken = m_getProfilesResp->Profiles[1].token;
    TokenDigest();
    OnvifSoapResult result = soap_call___trt__AddVideoSourceConfiguration(m_soap, m_mediaServerAddr.c_str(), NULL, m_addVideoSourceReq, m_addVideoSourceResp);
    return result;
}

void OnvifMedia::InitReqResp()
{
    m_getProfilesReq = (ProfilesReq*)soap_malloc(m_soap, sizeof(ProfilesReq));
    m_getProfilesResp = (ProfilesResp*)soap_malloc(m_soap, sizeof(ProfilesResp));

    m_getStreamUriReq = (StreamUriReq*)soap_malloc(m_soap, sizeof(StreamUriReq));
    m_getStreamUriResp = (StreamUriResp*)soap_malloc(m_soap, sizeof(StreamUriResp));
    m_getStreamUriReq->StreamSetup = (struct tt__StreamSetup*)soap_malloc(m_soap,sizeof(struct tt__StreamSetup));
    m_getStreamUriReq->StreamSetup->Transport = (struct tt__Transport *)soap_malloc(m_soap, sizeof(struct tt__Transport));

    m_startMulticastReq = (StartMulticastReq*)soap_malloc(m_soap, sizeof(StartMulticastReq));
    m_startMulticastResp = (StartMulticastResp*)soap_malloc(m_soap, sizeof(StartMulticastResp));
    m_stopMulticastReq = (StopMulticastReq*)soap_malloc(m_soap, sizeof(StopMulticastReq));
    m_stopMulticastResp = (StopMulticastResp*)soap_malloc(m_soap, sizeof(StopMulticastResp));

    m_setConfigOptReq = (SetVideoEncoderConfigReq*)soap_malloc(m_soap, sizeof(SetVideoEncoderConfigReq));
    m_setConfigOptResp = (SetVideoEncoderConfigResp*)soap_malloc(m_soap, sizeof(SetVideoEncoderConfigResp));

    m_getOSDsReq = (GetOSDsConfig*)soap_malloc(m_soap, sizeof(GetOSDsConfig));
    m_getOSDsResp = (GetOSDsConfigResp*)soap_malloc(m_soap, sizeof(GetOSDsConfigResp));

    m_createOSDReq = (CreateOSDConfig*)soap_malloc(m_soap, sizeof(CreateOSDConfig));
    m_createOSDResp = (CreateOSDConfigResp*)soap_malloc(m_soap, sizeof(CreateOSDConfigResp));

    m_deleteOSDReq = (DeleteOSDConfig*)soap_malloc(m_soap, sizeof(DeleteOSDConfig));
    m_deleteOSDResp = (DeleteOSDConfigResp*)soap_malloc(m_soap, sizeof(DeleteOSDConfigResp));

    m_setOSDReq = (SetOSDConfig*)soap_malloc(m_soap, sizeof(SetOSDConfig));
    m_setOSDResp = (SetOSDConfigResp*)soap_malloc(m_soap, sizeof(SetOSDConfigResp));

    m_addVideoSourceReq = (AddVideoSource*)soap_malloc(m_soap, sizeof(AddVideoSource));
    m_addVideoSourceResp = (AddVideoSourceResp*)soap_malloc(m_soap, sizeof(AddVideoSourceResp));
}

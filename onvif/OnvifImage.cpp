#include "OnvifImage.h"
#include "soap/gsoap/plugin/wsseapi.h"

OnvifImage::OnvifImage(const std::string &imageServername, SoapAuthentication auth, bool bIsConnect)
    : OnvifBasic(auth, bIsConnect)
    , m_imageServerAddr(imageServername)
{
    InitReqResp();
}

GetImagingResp *OnvifImage::GetImagingSetting(char* token)
{
    soap_default__timg__GetImagingSettings(m_soap, m_getImage);
    soap_default__timg__GetImagingSettingsResponse(m_soap, m_getImageResp);

    //source token
    m_getImage->VideoSourceToken = token;
    TokenDigest();
    int result = soap_call___timg__GetImagingSettings(m_soap, m_imageServerAddr.c_str(), NULL, m_getImage, m_getImageResp);
    if (result == SOAP_OK)
        return m_getImageResp;
}

OnvifSoapResult OnvifImage::SetImagingSetting(char *token, OnvifStreamArguments &args)
{
    //getImagingSetting(token);

    soap_default__timg__SetImagingSettings(m_soap, m_setImage);
    soap_default__timg__SetImagingSettingsResponse(m_soap, m_setImageResp);

    m_setImage->VideoSourceToken = token;
    float brightness = (float)args.imageArg.brightness;
    float saturation = (float)args.imageArg.saturation;
    float contrast = (float)args.imageArg.contrast;
    m_getImageResp->ImagingSettings->Brightness = &brightness;
    m_getImageResp->ImagingSettings->ColorSaturation = &saturation;
    m_getImageResp->ImagingSettings->Contrast = &contrast;

    m_setImage->ImagingSettings = m_getImageResp->ImagingSettings;
    TokenDigest();
    int result = soap_call___timg__SetImagingSettings(m_soap, m_imageServerAddr.c_str(), NULL, m_setImage, m_setImageResp);
    return result;
}

void OnvifImage::InitReqResp()
{
    m_getImage = (GetImagingReq*)soap_malloc(m_soap, sizeof(GetImagingReq));
    m_getImageResp = (GetImagingResp*)soap_malloc(m_soap, sizeof(GetImagingResp));

    m_setImage = (SetImagingReq*)soap_malloc(m_soap, sizeof(SetImagingReq));
    m_setImageResp = (SetImagingResp*)soap_malloc(m_soap, sizeof(SetImagingResp));
}

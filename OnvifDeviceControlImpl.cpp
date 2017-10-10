#include "OnvifDeviceControlImpl.h"

#include <iconv.h>
#include <swift/thread/engine_runner.hpp>

#include "OnvifDeviceControl.h"
#include "OnvifDeviceHelper.h"

#include "../../../../share/LocalAddressSelector.h"

swift::mutex OnvifDeviceControlImpl::s_devLock;
OnvifDeviceControlImpl::OnvifDeviceControlImpl(OnvifDeviceControlPtr pContorl, OnvifDevicePtr &pDevice, DLPCreateClassContextPtr &pCreateClassContext)
        : _DLPMulticastableStreams(m_lock, pCreateClassContext)
        , m_pControl(pContorl)
        , m_pDevice(pDevice)
        , m_OSDSetting(new OnvifOSDsArguments)
        , m_bIsMulticastOpened(false)
{
    string subserver = pDevice->GetSubServerAddress(SOAP_NAMESPACE_OF_trt);
    if (!subserver.empty())
        m_pMedia = OnvifMediaPtr(new OnvifMedia(subserver, pDevice->GetAuthInfo(), pDevice->IsConnect()));

    subserver = pDevice->GetSubServerAddress(SOAP_NAMESPACE_OF_tptz);
    if (!subserver.empty())
        m_pPtz = OnvifPTZPtr(new OnvifPTZ(subserver, pDevice->GetAuthInfo(), pDevice->IsConnect()));

    subserver = pDevice->GetSubServerAddress(SOAP_NAMESPACE_OF_timg);
    if (!subserver.empty())
        m_pImage = OnvifImagePtr(new OnvifImage(subserver, pDevice->GetAuthInfo(), pDevice->IsConnect()));

}

OnvifDeviceControlImpl::~OnvifDeviceControlImpl()
{
    delete m_OSDSetting;
    m_OSDSetting = NULL;
}

DP_RESULT OnvifDeviceControlImpl::GetControl(IDLPDeviceControl **ppControl)
{
    return DLR_S_OK;
}

void OnvifDeviceControlImpl::DoDisconnect(bool active)
{

}

void OnvifDeviceControlImpl::DoConfigure(const ConfigParams &params, IDLPContext *pContext)
{
    swift::mutex::lock_guard lock(m_lock);

    OnvifCfgParamsMap cfgMap;
    ConfigParams successParams;
    ConfigParams succDLVideoPartParams;
    ConfigParams succDLOSDPartParams;
    ConfigParams failureParams;
    OnvifVideoArguments getArgs(CFG_ACTION_GET);
    OnvifVideoArguments setArgs(CFG_ACTION_SET);

    SortParams(params, cfgMap);

    int actioncnt = cfgMap.find(ONVIF_ACTION_GET_VIDEO)->second.GetActionCount();
    if (actioncnt > 0) {
        DoVideoConfigGettings(getArgs);
        for (int i = 0; i < actioncnt; i++) {
            ProcessVideoGetActions(cfgMap.find(ONVIF_ACTION_GET_VIDEO)->second.GetAction(i), successParams, failureParams, getArgs);
        }
    }

    actioncnt = cfgMap.find(ONVIF_ACTION_SET_VIDEO)->second.GetActionCount();
    if (actioncnt > 0) {
        for (int i = 0; i < actioncnt; i++) {
            ProcessVideoSetActions(cfgMap.find(ONVIF_ACTION_SET_VIDEO)->second.GetAction(i), succDLVideoPartParams, failureParams, setArgs);
        }
        DoVideoConfigSettings(succDLVideoPartParams, successParams, failureParams, setArgs);
    }
#if 0
    actioncnt = cfgMap.find(ONVIF_ACTION_SET_NTP)->second.GetActionCount();
    if (actioncnt > 0)
            ProcessNTPSetActions(cfgMap.find(ONVIF_ACTION_SET_NTP)->second, pContext);
#endif
    actioncnt = cfgMap.find(ONVIF_ACTION_GET_OSD)->second.GetActionCount();
    if (actioncnt > 0) {
        DoOSDConfigGettings(cfgMap.find(ONVIF_ACTION_GET_OSD)->second.GetAction(0));
        for (int i = 0; i < actioncnt; i++) {
            ProcessOSDGetActions(cfgMap.find(ONVIF_ACTION_GET_OSD)->second.GetAction(i), successParams, failureParams);
        }
    }

    actioncnt = cfgMap.find(ONVIF_ACTION_SET_OSD)->second.GetActionCount();
    if (actioncnt > 0) {
        for (int i = 0; i < actioncnt; i++) {
            ProcessOSDSetActions(cfgMap.find(ONVIF_ACTION_SET_OSD)->second.GetAction(i), succDLOSDPartParams, failureParams);
        }
        DoOSDConfigSettings(succDLOSDPartParams, successParams, failureParams);
    }

    actioncnt = cfgMap.find(ONVIF_ACTION_INVOKE)->second.GetActionCount();
    if (actioncnt > 0) {
        for (int i = 0; i < actioncnt; i++) {
            ProcessInvokeActions(cfgMap.find(ONVIF_ACTION_INVOKE)->second.GetAction(i), successParams, failureParams);
        }
    }

    actioncnt = cfgMap.find(ONVIF_ACTION_NONE)->second.GetActionCount();
    if (actioncnt > 0) {
        for (int i = 0; i < actioncnt; i++) {
            ProcessInvalidActions(cfgMap.find(ONVIF_ACTION_NONE)->second.GetAction(i), failureParams);
        }
    }

    IDLPConfigCallback* pCallback = m_pControl->GetSafeConfigCallback();
    if(pCallback) {
        if (failureParams.GetActionCount() > 0)
            pCallback->OnConfigureComplete(static_cast<IDLPConfig*>(m_pControl.GetPtr()), failureParams, pContext);

        if (successParams.GetActionCount() > 0)
            pCallback->OnConfigureComplete(static_cast<IDLPConfig*>(m_pControl.GetPtr()), successParams, pContext);

        pCallback->Release();
    }
}

void OnvifDeviceControlImpl::DoControl(PTZ_ACTION action)
{
    swift::mutex::lock_guard lock(m_lock);
    bool bGohome;
    PTZPantilt pantilt;
    PTZZoom zoom;

    switch(action)
    {
    case PTZ_PAN_LEFT:
        pantilt.x = PTZ_MOVE_SPEED;
        break;
    case PTZ_PAN_RIGHT:
        pantilt.x = -PTZ_MOVE_SPEED;
        break;
    case PTZ_TILT_UP:
        pantilt.y = PTZ_MOVE_SPEED;
        break;
    case PTZ_TILT_DOWN:
        pantilt.y = -PTZ_MOVE_SPEED;
        break;
    case PTZ_LEFTUP:
        pantilt.x = PTZ_MOVE_SPEED;
        pantilt.y = PTZ_MOVE_SPEED;
        break;
    case PTZ_LEFTDOWN:
        pantilt.x = PTZ_MOVE_SPEED;
        pantilt.y = -PTZ_MOVE_SPEED;
        break;
    case PTZ_RIGHTUP:
        pantilt.x = -PTZ_MOVE_SPEED;
        pantilt.y = PTZ_MOVE_SPEED;
        break;
    case PTZ_RIGHTDOWN:
        pantilt.x = -PTZ_MOVE_SPEED;
        pantilt.y = -PTZ_MOVE_SPEED;
        break;
    case PTZ_ZOOM_IN:
        zoom.x = PTZ_MOVE_SPEED;
        break;
    case PTZ_ZOOM_OUT:
        zoom.x = -PTZ_MOVE_SPEED;
        break;
    case PTZ_AUTOSCAN_ON:
        bGohome = true;
        break;
    case PTZ_IRIS_CLOSE:
        break;
    case PTZ_IRIS_OPEN:
        break;
    case PTZ_FOCUS_NEAR:
        break;
    case PTZ_FOCUS_FAR:
        break;
    case PTZ_MOVE_STOP:
        break;
    case PTZ_AUTOSCAN_OFF:
        break;
    case PTZ_ZOOM_STOP:
        break;
    case PTZ_IRIS_STOP:
        break;
    case PTZ_FOCUS_STOP:
        break;
    case PTZ_PRESET_SET:
        break;
    case PTZ_PRESET_LOAD:
        break;
    case PTZ_PRESET_CLEAR:
        break;
    case PTZ_AUX_ON:
        break;
    case PTZ_AUX_OFF:
        break;
    default:
        break;
    }

    ProfilesResp *profiles = m_pMedia->GetMediaProfiles();
    if (profiles != NULL) {
        if (pantilt.x || pantilt.y || zoom.x) {
            pantilt.space = profiles->Profiles->PTZConfiguration->DefaultContinuousPanTiltVelocitySpace;
            zoom.space = profiles->Profiles->PTZConfiguration->DefaultContinuousZoomVelocitySpace;
            m_pPtz->ContinuousMove(profiles->Profiles->token, &pantilt, &zoom);
        } else if (bGohome) {
            m_pPtz->GotoHomePosition(profiles->Profiles->token, profiles->Profiles->PTZConfiguration->DefaultPTZSpeed);
        } else {
            //TODO : IRIS FOCUS AUX AUTOSCAN
        }
    }
    return;
}

void OnvifDeviceControlImpl::DoOpenStream(const STREAM_ID &strmId, IDLPContext *pContext)
{
    swift::mutex::lock_guard lock(m_lock);
    DLPStreamsEx* pStreamsEx = dynamic_cast<DLPStreamsEx*>(m_pControl.GetPtr());
    SWIFT_ASSERT(pStreamsEx);
    IDLPStreamsCallback* pCallback = pStreamsEx->GetSafeStreamsCallback();
    SWIFT_ASSERT(pCallback);

    OnvifInputStreamPtr pStream;
    pStream = OnvifInputStreamPtr(new OnvifInputStream(strmId, DLPStreamsExPtr(m_pControl.GetPtr(), true), STREAM_TRANS_PROTO_UDP));

    if (pStream) {
        if (strmId == MAIN_STREAM_ID) {
            if (m_mainStreamURI.empty())
                m_mainStreamURI = m_pMedia->GetStreamURI(strmId);

            if (m_pMainStreamClient == NULL) {
                m_pMainStreamClient = OnvifRtspClientPtr(new OnvifRtspClient(m_mainStreamURI, pStream, m_pDevice->GetAuthInfo(), DeviceLayer::EngineInstance::Instance().GetEngine()));
            } else {
                m_pMainStreamClient->SetStream(pStream);
            }
            m_pMainStreamClient->OpenStream(pContext);
        } else if (strmId == SUB_STREAM_ID){
            if (m_subStreamURI.empty())
                m_subStreamURI = m_pMedia->GetStreamURI(strmId);

            if (m_pSubStreamClient == NULL) {
                m_pSubStreamClient = OnvifRtspClientPtr(new OnvifRtspClient(m_subStreamURI, pStream, m_pDevice->GetAuthInfo(), DeviceLayer::EngineInstance::Instance().GetEngine()));
            } else {
                m_pSubStreamClient->SetStream(pStream);
            }
            m_pSubStreamClient->OpenStream(pContext);
        }
    }
}

void OnvifDeviceControlImpl::DoCloseStream(const DLPStreamExPtr &pStream, IDLPContext *pContext)
{
    swift::mutex::lock_guard lock(m_lock);
    if (!pStream->IsOutput()) {
        if (static_cast<OnvifInputStream*>(pStream.GetPtr(false))->IsMainStream()) {
            m_pMainStreamClient->CloseStream(pContext);
        } else {
            m_pSubStreamClient->CloseStream(pContext);
        }
    } else {
        // NOT_SUPPORTED;
        // OnvifInputStream* pInputStream = static_cast<OnvifInputStream*>(pStream.GetPtr(false));
    }
}

DP_RESULT OnvifDeviceControlImpl::StartMulticast(const STREAM_ID &id, unsigned long multAddr, unsigned short destPort, STREAM_TRANS_PROTO proto, IDLPContext *pContext)
{
    return _DLPMulticastableStreams::StartMulticast(id, multAddr, destPort, proto, pContext);
}

DP_RESULT OnvifDeviceControlImpl::OpenMulticastStream(const STREAM_ID &strmId, STREAM_TRANS_PROTO proto, IDLPContext *pContext)
{
    return _DLPMulticastableStreams::OpenStream(strmId, proto, pContext);
}

DP_RESULT OnvifDeviceControlImpl::StopMulticast(const STREAM_ID &id, IDLPContext *pContext)
{
    return _DLPMulticastableStreams::StopMulticast(id, pContext);
}

DP_RESULT OnvifDeviceControlImpl::CloseMulticastStream(const DLPStreamExPtr& pStream, IDLPContext *pContext)
{
    return _DLPMulticastableStreams::CloseStream(pStream, pContext);
}

DP_RESULT OnvifDeviceControlImpl::DoStartMulticast(const STREAM_ID &id, unsigned long multAddr, unsigned short destPort, STREAM_TRANS_PROTO proto, IDLPContext *pContext)
{
    OnvifSoapResult result = SOAP_EOF;
    if (id == MAIN_STREAM_ID) {
        result = m_pMedia->SetMulticastEncoderConfig(multAddr, destPort);
        if (result != SOAP_OK)
            return DeviceLayer::DLR_E_FAIL;

        result = m_pMedia->StartMulticast();
        if (result != SOAP_OK)
            return DeviceLayer::DLR_E_FAIL;
    } else {
        result = m_pMedia->SetMulticastEncoderConfig(multAddr, destPort, false);
        if (result != SOAP_OK)
            return DeviceLayer::DLR_E_FAIL;

        result = m_pMedia->StartMulticast(false);
        if (result != SOAP_OK)
            return DeviceLayer::DLR_E_FAIL;
    }
    return DeviceLayer::DLR_S_OK;
}

DP_RESULT OnvifDeviceControlImpl::DoStopMulticast(const STREAM_ID &id, IDLPContext *pContext)
{
    ONVIF_DEVICE_UNUSED(pContext)
    // confuse ......
    if (id == MAIN_STREAM_ID) {
        OnvifSoapResult result = m_pMedia->StopMulticast();
        if (result != SOAP_OK)
            return DeviceLayer::DLR_E_FAIL;
    } else {
        OnvifSoapResult result = m_pMedia->StopMulticast(false);
        if (result != SOAP_OK)
            return DeviceLayer::DLR_E_FAIL;
    }
    return DeviceLayer::DLR_S_OK;
}

bool OnvifDeviceControlImpl::CheckStatusBeforeCreateMultiStrm(string *workIp, string *peerIp)
{
    if (!m_pDevice->IsConnect())
        return false;

    string _workIp = LocalAddressSelector::GetDefaultLocalAddress();
    if(workIp)
        *workIp = _workIp;

    if(_workIp.empty())
        return false;

    return true;
}

void OnvifDeviceControlImpl::OnPostRequestControlComplete(int channIndex, IDLPContext *pContext)
{
    IDLPPtzControl2Callback* pCallback = m_pControl->GetSafePtzControl2Callback();
    if(pCallback) {
        pCallback->OnRequestControlComplete(m_pControl.GetPtr(), channIndex, DeviceLayer::DLR_S_OK, pContext);
        pCallback->Release();
    }
}

void OnvifDeviceControlImpl::OnPostReleaseControlComplete(int channIndex, IDLPContext *pContext)
{
    IDLPPtzControl2Callback* pCallBack = m_pControl->GetSafePtzControl2Callback();
    if (pCallBack) {
        pCallBack->OnReleaseControlComplete(m_pControl.GetPtr(), channIndex, DeviceLayer::DLR_S_OK, pContext);
        pCallBack->Release();
    }
}

void OnvifDeviceControlImpl::SortParams(const ConfigParams &params, OnvifCfgParamsMap &cfgmap)
{
    ConfigParams getVideoParams;
    ConfigParams getOSDParams;
    ConfigParams setVideoParams;
    ConfigParams setOSDParams;
    ConfigParams setNTPParams;
    ConfigParams invokeParams;
    ConfigParams invalidParams;

    int actioncnt = params.GetActionCount();
    for (int i = 0; i < actioncnt; i++) {
        ConfigAction action = params.GetAction(i);
        if (!action.IsValid())
            continue;

        ConfigTarget target = action.GetTarget();
        if (!target.IsValid())
            continue;

        CONFIG_ACTION_TYPE type = target.GetType();
        string name = target.GetTarget();
        switch (type) {
        case CFG_ACTION_GET:
        {
            if (name == DLCT_BRIGHTNESS || name == DLCT_CONTRAST
                    || name == DLCT_SATURATION || name == DLCT_BITRATE
                    || name == DLCT_FRAMERATE || name == DLCT_RESOLUTION) {
                getVideoParams.AddAction(action);
            } else if (name == DLCT_OSD_CUSTOM_SHOW || name == DLCT_OSD_CUSTOM_POS
                       || name == DLCT_OSD_CUSTOM_TEXT || name == DLCT_OSD_DATE_SHOW
                       || name == DLCT_OSD_DATE_POS || name == DLCT_OSD_TIME_POS
                       || name == DLCT_OSD_TIME_SHOW) {
                getOSDParams.AddAction(action);
            } else {
                invalidParams.AddAction(action);
            }
            break;
        }
        case CFG_ACTION_SET:
        {
            if (name == DLCT_BRIGHTNESS || name == DLCT_CONTRAST
                    || name == DLCT_SATURATION || name == DLCT_BITRATE
                    || name == DLCT_FRAMERATE || name == DLCT_RESOLUTION) {
                setVideoParams.AddAction(action);
            }  else if (name == DLCT_OSD_CUSTOM_SHOW || name == DLCT_OSD_CUSTOM_POS
                        || name == DLCT_OSD_CUSTOM_TEXT || name == DLCT_OSD_DATE_SHOW
                        || name == DLCT_OSD_DATE_POS || name == DLCT_OSD_TIME_POS
                        || name == DLCT_OSD_TIME_SHOW) {
                setOSDParams.AddAction(action);
            } else if (name == DLCT_NTPSERVER || name == DLCT_NTP_ENABLED) {
                AddParamAction(setNTPParams, action, action.GetResult(), action.GetValue());
            } else {
                invalidParams.AddAction(action);
            }
            break;
        }
        case CFG_ACTION_INVOKE:
        {
            invokeParams.AddAction(action);
            break;
        }
        case CFG_ACTION_NONE:
        {
            invalidParams.AddAction(action);
            break;
        }
        default:
        {
            invalidParams.AddAction(action);
            break;
        }
        }
    }

    cfgmap[ONVIF_ACTION_NONE] = invalidParams;
    cfgmap[ONVIF_ACTION_GET_VIDEO] = getVideoParams;
    cfgmap[ONVIF_ACTION_GET_OSD] = getOSDParams;
    cfgmap[ONVIF_ACTION_SET_VIDEO] = setVideoParams;
    cfgmap[ONVIF_ACTION_SET_OSD] = setOSDParams;
    cfgmap[ONVIF_ACTION_SET_NTP] = setNTPParams;
    cfgmap[ONVIF_ACTION_INVOKE] = invokeParams;
}

void OnvifDeviceControlImpl::DoVideoConfigSettings(const ConfigParams &setparams, ConfigParams &successParams, ConfigParams &failureParams, OnvifVideoArguments &setArgs)
{
    int actioncnt = setparams.GetActionCount();
    OnvifSoapResult imageResult = SOAP_EOF;
    OnvifSoapResult mediaResult = SOAP_EOF;
    if (actioncnt > 0) {
        ConfigAction act = setparams.GetAction(0);
        int index = act.GetTarget().GetIndex();
        if (index % 2 == 0) {
            char *maintoken = m_pMedia->GetMediaProfiles()->Profiles->VideoSourceConfiguration->SourceToken;
            imageResult = m_pImage->SetImagingSetting(maintoken, setArgs.mainStream);
            mediaResult = m_pMedia->SetVideoEncoderConfig(setArgs.mainStream);
        } else {
            char *subtoken = m_pMedia->GetMediaProfiles()->Profiles[1].VideoSourceConfiguration->SourceToken;
            imageResult = m_pImage->SetImagingSetting(subtoken, setArgs.subStream);
            mediaResult = m_pMedia->SetVideoEncoderConfig(setArgs.subStream, false);
        }

        for (int i = 0; i < actioncnt; i++) {
            ConfigAction action = setparams.GetAction(i);
            ConfigValue val = action.GetValue();
            ConfigTarget target = action.GetTarget();
            string name = target.GetTarget();
            if (name == DLCT_BRIGHTNESS || name == DLCT_CONTRAST || name == DLCT_SATURATION) {
                if (imageResult == SOAP_OK) {
                    AddParamAction(successParams, action, DLR_S_OK, val);
                } else {
                    AddParamAction(failureParams, action, DLR_E_FAIL, val);
                }
            } else if (name == DLCT_BITRATE || name == DLCT_FRAMERATE || name == DLCT_RESOLUTION) {
                if (mediaResult == SOAP_OK) {
                    AddParamAction(successParams, action, DLR_S_OK, val);
                } else {
                    AddParamAction(failureParams, action, DLR_E_FAIL, val);
                }
            }
        }
    }
}

void OnvifDeviceControlImpl::DoVideoConfigGettings(OnvifVideoArguments &getArgs)
{
    ProfilesResp *resp = m_pMedia->GetMediaProfiles();
    OnvifMediaArguments mediaArgs;
    OnvifImageArguments imageArgs;
    OnvifMediaResolution resolution;
    OnvifStreamArguments stream;
    {
        char *token = resp->Profiles->VideoSourceConfiguration->SourceToken;
        ImageConfig *image = m_pImage->GetImagingSetting(token)->ImagingSettings;
        if (resp->Profiles->VideoEncoderConfiguration->Resolution != NULL) {
            resolution.width = resp->Profiles->VideoEncoderConfiguration->Resolution->Width;
            resolution.height = resp->Profiles->VideoEncoderConfiguration->Resolution->Height;
            mediaArgs.resolution = resolution;
        }

        if (resp->Profiles->VideoEncoderConfiguration->RateControl != NULL) {
            mediaArgs.bitrate =resp->Profiles->VideoEncoderConfiguration->RateControl->BitrateLimit;
            mediaArgs.framerate = resp->Profiles->VideoEncoderConfiguration->RateControl->FrameRateLimit;
        }

        if (image != NULL) {
            imageArgs.brightness = (int)*image->Brightness;
            imageArgs.contrast = (int)*image->Contrast;
            imageArgs.saturation = (int)*image->ColorSaturation;
        }

        stream.imageArg = imageArgs;
        stream.mediaArg = mediaArgs;
        getArgs.mainStream = stream;
    }

    {
        char *token = resp->Profiles[1].VideoSourceConfiguration->SourceToken;
        ImageConfig *image = m_pImage->GetImagingSetting(token)->ImagingSettings;
        if (resp->Profiles->VideoEncoderConfiguration->Resolution != NULL) {
            resolution.width = resp->Profiles[1].VideoEncoderConfiguration->Resolution->Width;
            resolution.height = resp->Profiles[1].VideoEncoderConfiguration->Resolution->Height;
            mediaArgs.resolution = resolution;
        }

        if (resp->Profiles->VideoEncoderConfiguration->RateControl != NULL) {
            mediaArgs.bitrate =resp->Profiles[1].VideoEncoderConfiguration->RateControl->BitrateLimit;
            mediaArgs.framerate = resp->Profiles[1].VideoEncoderConfiguration->RateControl->FrameRateLimit;
        }

        if (image != NULL) {
            imageArgs.brightness = (int)*image->Brightness;
            imageArgs.contrast = (int)*image->Contrast;
            imageArgs.saturation = (int)*image->ColorSaturation;
        }

        stream.imageArg = imageArgs;
        stream.mediaArg = mediaArgs;
        getArgs.subStream = stream;
    }
}

void OnvifDeviceControlImpl::ProcessVideoGetActions(const ConfigAction &action, ConfigParams &successParams, ConfigParams &failureParams, OnvifVideoArguments &getArgs)
{
    ConfigTarget target = action.GetTarget();
    string name = target.GetTarget();
    int index = target.GetIndex();
    DeviceLayer::ConfigValue val;
    DP_RESULT res = DLR_E_FAIL;
    if (name == DLCT_BRIGHTNESS || name == DLCT_CONTRAST || name == DLCT_SATURATION) {
        if (name == DLCT_BRIGHTNESS) {
            res = TransImageSetting(val, index %2 == 0 ? getArgs.mainStream.imageArg.brightness : getArgs.subStream.imageArg.brightness, false);
        } else if (name == DLCT_CONTRAST) {
            res = TransImageSetting(val, index %2 == 0 ? getArgs.mainStream.imageArg.contrast : getArgs.subStream.imageArg.contrast, false);
        } else {
            res = TransImageSetting(val, index %2 == 0 ? getArgs.mainStream.imageArg.saturation : getArgs.subStream.imageArg.saturation, false);
        }
    } else if(name == DLCT_BITRATE || name == DLCT_FRAMERATE || name == DLCT_RESOLUTION) {
        if (name == DLCT_BITRATE) {
            res = TransBitrateValue(val, index %2 == 0 ? getArgs.mainStream.mediaArg.bitrate : getArgs.subStream.mediaArg.bitrate, false);
        } else if (name == DLCT_FRAMERATE) {
            res = TransFramerateValue(val, index %2 == 0 ? getArgs.mainStream.mediaArg.framerate : getArgs.subStream.mediaArg.framerate, false);
        } else {
            res = TransResolutionValue(val, index %2 == 0 ? getArgs.mainStream.mediaArg.resolution : getArgs.subStream.mediaArg.resolution, false);
        }
    }
    if (res == DLR_S_OK) {
        AddParamAction(successParams, action, res, val);
    } else {
        AddParamAction(failureParams, action, res, val);
    }
}

void OnvifDeviceControlImpl::ProcessVideoSetActions(const ConfigAction &action, ConfigParams &successParams, ConfigParams &failureParams, OnvifVideoArguments &setArgs)
{
    ConfigTarget target = action.GetTarget();
    string name = target.GetTarget();
    ConfigValue val = action.GetValue();
    int index = target.GetIndex();
    DLResult res = DLR_E_FAIL;

    if (name == DLCT_BRIGHTNESS || name == DLCT_CONTRAST || name == DLCT_SATURATION) {
        if (name == DLCT_BRIGHTNESS ) {
            res = TransImageSetting(val, index %2 == 0 ? setArgs.mainStream.imageArg.brightness : setArgs.subStream.imageArg.brightness, true);
        } else if (name == DLCT_CONTRAST) {
            res = TransImageSetting(val, index %2 == 0 ? setArgs.mainStream.imageArg.contrast : setArgs.subStream.imageArg.contrast, true);
        } else {
            res = TransImageSetting(val, index %2 == 0 ? setArgs.mainStream.imageArg.saturation : setArgs.subStream.imageArg.saturation, true);
        }
    } else if (name == DLCT_BITRATE || name == DLCT_FRAMERATE || name == DLCT_RESOLUTION) {
        if (name == DLCT_BITRATE) {
            res = TransBitrateValue(val, index %2 == 0 ? setArgs.mainStream.mediaArg.bitrate : setArgs.subStream.mediaArg.bitrate, true);
        } else if (name == DLCT_FRAMERATE) {
            res = TransFramerateValue(val, index %2 == 0 ? setArgs.mainStream.mediaArg.framerate : setArgs.subStream.mediaArg.framerate, true);
        } else {
            res = TransResolutionValue(val, index %2 == 0 ? setArgs.mainStream.mediaArg.resolution : setArgs.subStream.mediaArg.resolution, true);
        }
    }

    if (res == DLR_S_OK) {
        AddParamAction(successParams, action, res, val);
    } else {
        AddParamAction(failureParams, action, res, val);
    }
}

void OnvifDeviceControlImpl::DoOSDConfigSettings(const ConfigParams &setparams, ConfigParams &successParams, ConfigParams &failureParams)
{
    int actioncnt = setparams.GetActionCount();
    if (actioncnt > 0) {
        ConfigAction action = setparams.GetAction(0);
        int index = action.GetTarget().GetIndex();

        OnvifSoapResult datetimeRes = SOAP_EOF;
        OnvifSoapResult plainTextRes = SOAP_EOF;
        if (index % 2 == 0) {
            datetimeRes = DoEachStreamOSDDataTimeSettings(m_OSDSetting->mainOSDArg);
            plainTextRes = DoEachStreamOSDPlanTextSettings(m_OSDSetting->mainOSDArg);
        } else {
            datetimeRes = DoEachStreamOSDDataTimeSettings(m_OSDSetting->subOSDArg, false);
            plainTextRes = DoEachStreamOSDPlanTextSettings(m_OSDSetting->subOSDArg, false);
        }
        for (int i = 0; i < actioncnt; i++) {
            ConfigAction action = setparams.GetAction(i);
            ConfigValue val = action.GetValue();
            ConfigTarget target = action.GetTarget();
            string name = target.GetTarget();
            if (name == DLCT_OSD_DATE_SHOW || name == DLCT_OSD_TIME_SHOW
                    || name == DLCT_OSD_DATE_POS || name == DLCT_OSD_TIME_POS ) {
                if (datetimeRes == SOAP_OK) {
                    AddParamAction(successParams, action, DLR_S_OK, val);
                } else {
                    AddParamAction(failureParams, action, DLR_E_FAIL, val);
                }
            } else if (name == DLCT_OSD_CUSTOM_SHOW || name == DLCT_OSD_CUSTOM_POS
                       || name == DLCT_OSD_CUSTOM_TEXT) {
                if (plainTextRes == SOAP_OK) {
                    AddParamAction(successParams, action, DLR_S_OK, val);
                } else {
                    AddParamAction(failureParams, action, DLR_E_FAIL, val);
                }
            }
        }
    }
}

OnvifSoapResult OnvifDeviceControlImpl::DoEachStreamOSDDataTimeSettings(OnvifOSDArguments *args, bool bIsMainStream)
{
    OnvifSoapResult res = SOAP_EOF;

    switch (args->setDateTimeCondition) {
    case LAST_CONDITION_NONE:
    {
        res = SOAP_OK;
        break;
    }
    case LAST_CONDITION_SET:
    {
        res = m_pMedia->SetOSD(args, bIsMainStream);
        break;
    }
    case LAST_CONDITION_CREATE:
    {
        res = m_pMedia->CreateOSD(args, bIsMainStream);
        break;
    }
    case LAST_CONDITION_DELETE:
    {
        res = m_pMedia->DeleteOSD(args, bIsMainStream);
        break;
    }
    default:
        break;
    }

    return res;
}

OnvifSoapResult OnvifDeviceControlImpl::DoEachStreamOSDPlanTextSettings(OnvifOSDArguments *args, bool bIsMainStream)
{
    OnvifSoapResult res = SOAP_EOF;

    switch (args->setCustomTextCondition) {
    case LAST_CONDITION_NONE:
    {
        res = SOAP_OK;
        break;
    }
    case LAST_CONDITION_SET:
    {
        res = m_pMedia->SetOSD(args, bIsMainStream, false);
        break;
    }
    case LAST_CONDITION_CREATE:
    {
        res = m_pMedia->CreateOSD(args, bIsMainStream, false);
        break;
    }
    case LAST_CONDITION_DELETE:
    {
        res = m_pMedia->DeleteOSD(args, bIsMainStream, false);
        break;
    }
    default:
        break;
    }

    return res;
}

void OnvifDeviceControlImpl::DoOSDConfigGettings(const ConfigAction &action)
{
    ConfigTarget target = action.GetTarget();
    int index = target.GetIndex();
    if (index % 2 == 0) {
        DoEachStremOSDGetting(m_OSDSetting->mainOSDArg);
    } else {
        DoEachStremOSDGetting(m_OSDSetting->subOSDArg);
    }
}

void OnvifDeviceControlImpl::DoEachStremOSDGetting(OnvifOSDArguments *args, bool bIsMainStream)
{
    GetOSDsConfigResp *osds;
    if (bIsMainStream) {
        osds = m_pMedia->GetOSDs();
    } else {
        osds = m_pMedia->GetOSDs(false);
    }
    if (osds != NULL) {
        bool isGetPlainComplete = false;
        bool isGetDatetimeComplete = false;
        for(int i = 0; i < osds->__sizeOSDs; i++) {
            if (!strcmp(osds->OSDs[i].TextString->Type, OSD_TYPE_DATATIME)) {
                args->bShowDateTime = true;
                args->datetimeToken = osds->OSDs[i].token;
                args->datetimePos->x = osds->OSDs[i].Position->Pos->x;
                args->datetimePos->y = osds->OSDs[i].Position->Pos->y;
                isGetDatetimeComplete = true;
                continue;
            }
            if (!strcmp(osds->OSDs[i].TextString->Type, OSD_TYPE_PLAINTEXT)) {
                if (isGetPlainComplete && isGetDatetimeComplete)
                        break;
                if (!isGetPlainComplete) {
                    args->bShowCustomText = true;
                    args->customToken = osds->OSDs[i].token;
                    strcpy(args->customText, osds->OSDs[i].TextString->PlainText);
                    args->customPos->x = osds->OSDs[i].Position->Pos->x;
                    args->customPos->y = osds->OSDs[i].Position->Pos->y;
                    isGetPlainComplete = true;
                }
            }
        }
    }
}

void OnvifDeviceControlImpl::ProcessOSDGetActions(const ConfigAction &action, ConfigParams &successParams, ConfigParams &failureParams)
{
    string name = action.GetTarget().GetTarget();
    ConfigValue val;
    int index = action.GetTarget().GetIndex();

    if (index % 2 == 0) {
        if (ProcessEachStreamOSDGetting(name, val, m_OSDSetting->mainOSDArg)) {
            AddParamAction(successParams, action, DLR_S_OK, val);
        } else {
            AddParamAction(failureParams, action, DLR_E_FAIL, val);
        }
    } else {
        if (ProcessEachStreamOSDGetting(name, val, m_OSDSetting->subOSDArg)) {
            AddParamAction(successParams, action, DLR_S_OK, val);
        } else {
            AddParamAction(failureParams, action, DLR_E_FAIL, val);
        }
    }
}

bool OnvifDeviceControlImpl::ProcessEachStreamOSDGetting(const string &name, ConfigValue &cfgval, OnvifOSDArguments *args)
{
    DLResult res = DLR_E_FAIL;
    if (name == DLCT_OSD_CUSTOM_SHOW) {
        if (args->bShowCustomText) {
            cfgval = 1;
            res = DLR_S_OK;
        }
    } else if (name == DLCT_OSD_CUSTOM_POS) {
            res = TransOSDPosition(cfgval, args->customPos);
    } else if (name == DLCT_OSD_CUSTOM_TEXT) {
        if (args->customText != NULL) {
            char tmp[90];
            ConvertEncodingmode("GBK","UTF-8", args->customText, 90
                                , tmp, 90);
            cfgval = tmp;
            res = DLR_S_OK;
        }
    } else if (name == DLCT_OSD_DATE_SHOW || name == DLCT_OSD_TIME_SHOW) {
        if (args->bShowDateTime) {
            cfgval = 1;
            res = DLR_S_OK;
        }
    } else if (name == DLCT_OSD_DATE_POS || name == DLCT_OSD_TIME_POS) {
            res = TransOSDPosition(cfgval, args->datetimePos);
    }
    return !res ? true : false;
}

void OnvifDeviceControlImpl::ProcessOSDSetActions(const ConfigAction &action, ConfigParams &successParams, ConfigParams &failureParams)
{
    ConfigValue val = action.GetValue();
    string name = action.GetTarget().GetTarget();
    int index = action.GetTarget().GetIndex();

    if (index % 2 == 0) {
        if (ProcessEachStreamOSDSetting(name, val, m_OSDSetting->mainOSDArg)) {
            AddParamAction(successParams, action, DLR_S_OK, val);
        } else {
            AddParamAction(failureParams, action, DLR_E_FAIL, val);
        }
    } else {
        if (ProcessEachStreamOSDSetting(name, val, m_OSDSetting->subOSDArg)) {
            AddParamAction(successParams, action, DLR_S_OK, val);
        } else {
            AddParamAction(failureParams, action, DLR_E_FAIL, val);
        }
    }
}

bool OnvifDeviceControlImpl::ProcessEachStreamOSDSetting(const string &name, ConfigValue &cfgval, OnvifOSDArguments *args)
{
    DLResult res = DLR_E_FAIL;
    if (name == DLCT_OSD_CUSTOM_SHOW) {
        if (cfgval.ToInt() == 1) {
            if (args->bShowCustomText) {
                args->setCustomTextCondition = LAST_CONDITION_SET;
            } else {
                args->setCustomTextCondition = LAST_CONDITION_CREATE;
            }
        } else if (cfgval.ToInt() == 0) {
            if (args->bShowCustomText) {
                args->setCustomTextCondition = LAST_CONDITION_DELETE;
            } else {
                args->setCustomTextCondition = LAST_CONDITION_NONE;
            }
        }
        res = DLR_S_OK;
    } else if (name == DLCT_OSD_CUSTOM_POS) {
        res = TransOSDPosition(cfgval, args->customPos, true);
    } else if (name == DLCT_OSD_CUSTOM_TEXT) {
        char tmpin[90];
        char tmpout[90];
        strcpy(tmpin, cfgval.ToString());
        ConvertEncodingmode("UTF-8","GBK", tmpin, (int)strlen(tmpin)
                            , tmpout, 90);
        strcpy(args->customText, tmpout);
        res = DLR_S_OK;
    } else if (name == DLCT_OSD_DATE_SHOW || name == DLCT_OSD_TIME_SHOW) {
        if (cfgval.ToInt() == 1) {
            if (args->bShowDateTime) {
                args->setDateTimeCondition = LAST_CONDITION_SET;
            } else {
                args->setDateTimeCondition = LAST_CONDITION_CREATE;
            }
        } else if (cfgval.ToInt() == 0) {
            if (args->bShowDateTime) {
                args->setDateTimeCondition = LAST_CONDITION_DELETE;
            } else {
                args->setDateTimeCondition = LAST_CONDITION_NONE;
            }
        }
    } else if (name == DLCT_OSD_DATE_POS || name == DLCT_OSD_TIME_POS) {
        res = TransOSDPosition(cfgval, args->datetimePos, true);
    }
    return !res ? true : false;
}

void OnvifDeviceControlImpl::ProcessNTPSetActions(ConfigParams &ntpParams, IDLPContext *pContext)
{
    IDLPConfigCallback* pCallback = m_pControl->GetSafeConfigCallback();

    if(pCallback) {
        pCallback->OnConfigureComplete(static_cast<IDLPConfig*>(m_pControl.GetPtr()), ntpParams, pContext);
        pCallback->Release();
    }
}

void OnvifDeviceControlImpl::ProcessInvokeActions(const ConfigAction &action, ConfigParams &successParams, ConfigParams &failureParams)
{
#if 0
    string name = action.GetTarget().GetTarget();
    if (name == DLCT_SAVECONFIG){
        AddParamAction(failureParams, action, DLR_E_NOT_SUPPORTED);
    }
#endif
}

void OnvifDeviceControlImpl::ProcessInvalidActions(const ConfigAction &action, ConfigParams &failureParams)
{
    AddParamAction(failureParams, action, DLR_E_FAIL);
}

void OnvifDeviceControlImpl::AddParamAction(ConfigParams &params, const ConfigAction &action, DLResult res, const ConfigValue &val)
{
    DeviceLayer::ConfigAction action1 = action;
    action1.SetResult(res);
    action1.SetValue(val);
    params.AddAction(action1);
}

DP_RESULT OnvifDeviceControlImpl::TransImageSetting(ConfigValue &cfgVal, int &devVal, bool toDevVal)
{
    if (toDevVal) {
        devVal = DLPHelper::RangeTranslate(cfgVal.ToInt(), -100, 100, 0, 100);
        if (cfgVal.ToInt() <= 100 || cfgVal.ToInt() >= -100) {
            return DLR_S_OK;
        } else {
            return DLR_E_FAIL;
        }
    } else {
        cfgVal = DLPHelper::RangeTranslate(devVal, 0, 100, -100, 100);
        if (devVal >= 0 || devVal <= 100) {
            return DLR_S_OK;
        } else {
            return DLR_E_FAIL;
        }
    }
}

DP_RESULT OnvifDeviceControlImpl::TransBitrateValue(ConfigValue &cfgVal, int &devVal, bool toDevVal)
{
    if(toDevVal) {
        devVal = cfgVal.ToInt();
        if (devVal < 16 || devVal > 16384)
            return DLR_E_FAIL;
        return DLR_S_OK;
    } else {
        cfgVal = devVal;
        if (cfgVal.ToInt() < 16 || cfgVal.ToInt() > 16384)
            return DLR_E_FAIL;
        return DLR_S_OK;
    }
}

DP_RESULT OnvifDeviceControlImpl::TransFramerateValue(ConfigValue &cfgVal, int &devVal, bool toDevVal)
{
    typedef map<int, string> FramerateValues;
    static FramerateValues s_framerateVals;

    swift::mutex::lock_guard lock(s_devLock);
    if (s_framerateVals.empty()) {
        s_framerateVals.insert(pair<int, string>(0, "25"));
        s_framerateVals.insert(pair<int, string>(5, "1"));
        s_framerateVals.insert(pair<int, string>(6, "2"));
        s_framerateVals.insert(pair<int, string>(7, "4"));
        s_framerateVals.insert(pair<int, string>(8, "6"));
        s_framerateVals.insert(pair<int, string>(9, "8"));
        s_framerateVals.insert(pair<int, string>(10, "10"));
        s_framerateVals.insert(pair<int, string>(11, "12"));
        s_framerateVals.insert(pair<int, string>(12, "16"));
        s_framerateVals.insert(pair<int, string>(13, "20"));
        s_framerateVals.insert(pair<int, string>(14, "15"));
        s_framerateVals.insert(pair<int, string>(15, "18"));
        s_framerateVals.insert(pair<int, string>(16, "22"));
    }

    if (toDevVal) {
        int framerate = cfgVal.ToInt();
        FramerateValues::iterator it = s_framerateVals.begin();
        for (; it != s_framerateVals.end(); it++) {
            if (framerate == it->first) {
                devVal = atoi(it->second.c_str());
                return DLR_S_OK;
            }
        }
        return DLR_E_FAIL;
    } else {
        FramerateValues::iterator it = s_framerateVals.begin();
        for (; it != s_framerateVals.end(); it++) {
            if (atoi(it->second.c_str()) == devVal) {
                cfgVal = it->first;
                return DLR_S_OK;
            }
        }
        return DLR_E_FAIL;
    }
}

DP_RESULT OnvifDeviceControlImpl::TransResolutionValue(ConfigValue &cfgVal, OnvifMediaResolution &resolution, bool toDevVal)
{
    typedef vector<string> ResolutionValues;
    static ResolutionValues s_devVals;

    swift::mutex::lock_guard lock(s_devLock);
    if (s_devVals.empty()) {
        s_devVals.push_back(MAKERESO(2560, 1920));
        s_devVals.push_back(MAKERESO(2448, 2048));
        s_devVals.push_back(MAKERESO(2048, 1536));
        s_devVals.push_back(MAKERESO(2448, 1200));
        s_devVals.push_back(MAKERESO(2448, 800));
        s_devVals.push_back(MAKERESO(1920, 1080));
        s_devVals.push_back(MAKERESO(1920, 540));
        s_devVals.push_back(MAKERESO(1600, 1200));
        s_devVals.push_back(MAKERESO(1600, 304));
        s_devVals.push_back(MAKERESO(1440, 900));
        s_devVals.push_back(MAKERESO(1280, 1024));
        s_devVals.push_back(MAKERESO(1280, 960));
        s_devVals.push_back(MAKERESO(1280, 900));
        s_devVals.push_back(MAKERESO(1280, 720));
        s_devVals.push_back(MAKERESO(1024, 768));
        s_devVals.push_back(MAKERESO(960, 576));
        s_devVals.push_back(MAKERESO(960, 540));
        s_devVals.push_back(MAKERESO(800, 600));
        s_devVals.push_back(MAKERESO(704, 576));
        s_devVals.push_back(MAKERESO(704, 288));
        s_devVals.push_back(MAKERESO(640, 480));
        s_devVals.push_back(MAKERESO(528, 384));
        s_devVals.push_back(MAKERESO(352, 288));
        s_devVals.push_back(MAKERESO(320, 240));
        s_devVals.push_back(MAKERESO(176, 144));
        s_devVals.push_back(MAKERESO(160, 120));
    }
    if (toDevVal) {
        string val = cfgVal.ToString();
        ResolutionValues::const_iterator it = s_devVals.begin();
        for (; it != s_devVals.end(); it++) {
            if (*it== val) {
                string width = val.substr(0, val.find("x"));
                string height = val.substr(val.find("x")+1, val.length());
                resolution.width = atoi(width.c_str());
                resolution.height = atoi(height.c_str());
                return DLR_S_OK;
            }
        }
        return DLR_E_FAIL;
    } else {
        char val[10];
        sprintf(val, "%dx%d", resolution.width, resolution.height);
        cfgVal = val;
        ResolutionValues::const_iterator it = s_devVals.begin();
        for (; it != s_devVals.end(); it++) {
            if (*it == cfgVal.ToString())
                return DLR_S_OK;
        }
        return DLR_E_FAIL;
    }
}

DP_RESULT OnvifDeviceControlImpl::TransOSDPosition(ConfigValue &cfgVal, OnvifOSDPosition *devVal, bool toDevVal)
{
    if (toDevVal) {
        string val = cfgVal.ToString();
        string sPosX = val.substr(0, val.find(","));
        string sPosY = val.substr(val.find(",")+1, val.length());
        int iPosX = atoi(sPosX.c_str());
        int iPosY = atoi(sPosY.c_str());
        *devVal->x= TransCoordinateValue((float)iPosX, 0, 1000, -1, 1);
        *devVal->y= TransCoordinateValue((float)iPosY, 1000, 0, -1, 1);
        if ((*devVal->x > 1 || *devVal->x < -1) || (*devVal->y > 1|| *devVal->y < -1))
            return DLR_E_FAIL;
        return DLR_S_OK;
    } else {
        char val[8];
        int iPosX = (int)TransCoordinateValue(*devVal->x, -1, 1, 0, 1000);
        int iPosY = (int)TransCoordinateValue(*devVal->y, -1, 1, 1000, 0);
        if ((iPosX > 1000 || iPosX < 0) || (iPosY > 1000 || iPosY < 0))
            return DLR_E_FAIL;
        sprintf(val, "%d,%d", iPosX, iPosY);
        cfgVal = val;
        return DLR_S_OK;
    }
}

int OnvifDeviceControlImpl::ConvertEncodingmode(const char *dest, const char *src, char *input, size_t ilen, char *output, size_t olen)
{
    char **inbuf = &input;
    char **outbuf = &output;
    if(dest == NULL || src == NULL)
            return -1;
    iconv_t conv = iconv_open(dest, src);
    if (conv == (iconv_t) -1)
            return -1;
    memset(output, 0, olen);
    if (iconv(conv, inbuf, &ilen, outbuf, &olen))
            return -1;
    iconv_close(conv);
    return 0;
}

float OnvifDeviceControlImpl::TransCoordinateValue(const float &val, const int &valRangeMin, const int &valRangeMax, const int &convRangeMin, const int &convRangeMax)
{
    if (val == OSD_NOT_EXISTENT)
        return 0;

    float res = (float)((val - valRangeMin) * (convRangeMax - convRangeMin) / (valRangeMax - valRangeMin) + convRangeMin);
    //float value = ((float)((int)(res*100)))/100;
    return res;
}

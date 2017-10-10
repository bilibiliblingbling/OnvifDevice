#ifndef __ONVIF_DEVICE_PLUGIN_CONTROLIMPL_H__
#define __ONVIF_DEVICE_PLUGIN_CONTROLIMPL_H__

#include "OnvifDeviceControl.h"
#include "OnvifDeviceConfigure.h"
#include "OnvifDeviceHelper.h"

#include "onvif/OnvifMedia.h"
#include "onvif/OnvifPTZ.h"
#include "onvif/OnvifImage.h"
#include "rtsp/OnvifRtspClient.h"

typedef std::map<ONVIF_ACTION_TYPE, ConfigParams> OnvifCfgParamsMap;
typedef DLPMulticastableStreams<DLPStreamsEx> _DLPMulticastableStreams;

class OnvifDeviceControlImpl : public _DLPMulticastableStreams
{
public:
    OnvifDeviceControlImpl(OnvifDeviceControlPtr pContorl, OnvifDevicePtr &pDevice, DLPCreateClassContextPtr &pCreateClassContext);
    ~OnvifDeviceControlImpl();

    DEFINE_OBJECT_INTERFACE()
public:
    virtual DP_RESULT DLPMETHOD GetControl(IDLPDeviceControl** ppControl);

    void DoDisconnect(bool active);
    void DoConfigure(const ConfigParams& params, IDLPContext* pContext);
    void DoControl(PTZ_ACTION action);
    void DoOpenStream(const STREAM_ID& strmId, IDLPContext* pContext);
    void DoCloseStream(const DLPStreamExPtr& pStream, IDLPContext* pContext);

    DP_RESULT StartMulticast(const STREAM_ID &id, unsigned long multAddr, unsigned short destPort, STREAM_TRANS_PROTO proto, IDLPContext *pContext);
    DP_RESULT OpenMulticastStream(const STREAM_ID& strmId, STREAM_TRANS_PROTO proto, IDLPContext* pContext);
    DP_RESULT StopMulticast(const STREAM_ID &id, IDLPContext *pContext);
    DP_RESULT CloseMulticastStream(const DLPStreamExPtr &pStream, IDLPContext *pContext);
    virtual DP_RESULT DoStartMulticast(const STREAM_ID &id, unsigned long multAddr, unsigned short destPort, STREAM_TRANS_PROTO proto, IDLPContext *pContext);
    virtual DP_RESULT DoStopMulticast(const STREAM_ID &id, IDLPContext *pContext);
    virtual bool CheckStatusBeforeCreateMultiStrm(string *workIp, string *peerIp);

private:
    void OnPostRequestControlComplete(int channIndex, IDLPContext* pContext);
    void OnPostReleaseControlComplete(int channIndex, IDLPContext* pContext);

    void SortParams(const ConfigParams& params, OnvifCfgParamsMap& cfgmap);

    void DoVideoConfigSettings(const ConfigParams& setparams, ConfigParams& successParams, ConfigParams& failureParams, OnvifVideoArguments& setArgs);
    void DoVideoConfigGettings(OnvifVideoArguments& getArgs);
    void ProcessVideoGetActions(const ConfigAction& action, ConfigParams& successParams, ConfigParams& failureParams, OnvifVideoArguments& getArgs);
    void ProcessVideoSetActions(const ConfigAction& action, ConfigParams& successParams, ConfigParams& failureParams, OnvifVideoArguments& setArgs);

    void DoOSDConfigSettings(const ConfigParams& setparams, ConfigParams& successParams, ConfigParams& failureParams);
    OnvifSoapResult DoEachStreamOSDDataTimeSettings(OnvifOSDArguments *args, bool bIsMainStream = true);
    OnvifSoapResult DoEachStreamOSDPlanTextSettings(OnvifOSDArguments *args, bool bIsMainStream = true);
    void DoOSDConfigGettings(const ConfigAction& action);
    void DoEachStremOSDGetting(OnvifOSDArguments *args, bool bIsMainStream = true);
    void ProcessOSDGetActions(const ConfigAction& action, ConfigParams& successParams, ConfigParams &failureParams);
    bool ProcessEachStreamOSDGetting(const string& name, ConfigValue& cfgval, OnvifOSDArguments *args);
    void ProcessOSDSetActions(const ConfigAction& action, ConfigParams& successParams, ConfigParams& failureParams);
    bool ProcessEachStreamOSDSetting(const string& name, ConfigValue& cfgval, OnvifOSDArguments *args);
    void ProcessNTPSetActions(ConfigParams &ntpParams, IDLPContext* pContext);

    void ProcessInvokeActions(const ConfigAction& action, ConfigParams& successParams, ConfigParams& failureParams);
    void ProcessInvalidActions(const ConfigAction& action, ConfigParams& failureParams);

    static void AddParamAction(ConfigParams& params, const ConfigAction& action, DLResult res, const ConfigValue& val = ConfigValue());
    static float TransCoordinateValue(const float& val, const int &valRangeMin, const int &valRangeMax, const int &convRangeMin, const int &convRangeMax);
    static DP_RESULT TransImageSetting(ConfigValue& cfgVal, int& devVal, bool toDevVal = false);
    static DP_RESULT TransBitrateValue(ConfigValue& cfgVal, int& devVal, bool toDevVal = false);
    static DP_RESULT TransFramerateValue(ConfigValue& cfgVal, int& devVal, bool toDevVal = false);
    static DP_RESULT TransResolutionValue(ConfigValue& cfgVal, OnvifMediaResolution& resolution, bool toDevVal = false);
    static DP_RESULT TransOSDPosition(ConfigValue& cfgVal, OnvifOSDPosition *devVal, bool toDevVal = false);
    static int ConvertEncodingmode(const char *dest, const char *src, char *input, size_t ilen, char *output, size_t olen);

public:
    bool m_bIsMulticastOpened;

    static swift::mutex s_devLock;
    swift::mutex m_lock;

    OnvifDeviceControlPtr m_pControl;
    OnvifDevicePtr m_pDevice;
    OnvifMediaPtr m_pMedia;
    OnvifImagePtr m_pImage;
    OnvifPTZPtr m_pPtz;

    OnvifOSDsArguments *m_OSDSetting;

    swift::net::rtsp::rtsp_url m_mainStreamURI;
    swift::net::rtsp::rtsp_url m_subStreamURI;
    OnvifRtspClientPtr m_pMainStreamClient;
    OnvifRtspClientPtr m_pSubStreamClient;
};

#endif // __ONVIF_DEVICE_PLUGIN_CONTROLIMPL_H__

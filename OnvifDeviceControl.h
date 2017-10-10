#ifndef __ONVIF_DEVICE_PLUGIN_CONTROL_H__
#define __ONVIF_DEVICE_PLUGIN_CONTROL_H__

#include "OnvifStream.h"
#include "onvif/OnvifDevice.h"

#include <swift/thread/engine_runner.hpp>

class OnvifDeviceControl;
typedef DeviceLayer::ObjectPtr<OnvifDeviceControl> OnvifDeviceControlPtr;

class OnvifDeviceControlImpl;

class OnvifDeviceControl : public DLPDeviceControlEx2
                         , public DLPConfig
                         , public DLPStreamsEx
                         , public DLPPtzControl2
{
public:
    OnvifDeviceControl(OnvifDevicePtr &onvifDevice, DLPCreateClassContextPtr &pCreateClassContext);
    ~OnvifDeviceControl();

public:
    DLPDEVCTRL_EX_IMPLEMENT_GET_CONTROL()
    DEFINE_OBJECT_INTERFACE()

    virtual DP_RESULT DLPMETHOD Disconnect();
    virtual DP_RESULT DLPMETHOD Configure(const ConfigParams& params, ConfigParams& failures, IDLPContext* pContext);
    virtual DP_RESULT DLPMETHOD OpenStream(const STREAM_ID& id, STREAM_TRANS_PROTO proto, IDLPContext* pContext);
    virtual DP_RESULT CloseStream(const DLPStreamExPtr& pStream, IDLPContext* pContext);
protected:
    virtual DP_RESULT DLPMETHOD StartMulticast(const STREAM_ID& id, unsigned long multAddr, unsigned short destPort, STREAM_TRANS_PROTO proto, IDLPContext* pContext);
    virtual DP_RESULT DLPMETHOD StopMulticast(const STREAM_ID& id, IDLPContext* pContext);

    virtual DP_RESULT DLPMETHOD RequestControl(int channIndex, IDLPContext* pContext);
    virtual DP_RESULT DLPMETHOD ReleaseControl(int channIndex, IDLPContext* pContext);
    virtual DP_RESULT DLPMETHOD Control(int channIndex, PTZ_ACTION action, const PtzActionParams& params);

protected:
    swift::mutex m_lock;
    swift::mutex m_underLock;

    // for PTZ
    swift::engine m_engine;
    swift::engine_runner m_underRunner;
private:
    OnvifDeviceControlImpl *m_p_;

};
#endif /// __ONVIF_HIK_DEVICE_PLUGIN_CONTROL_H__

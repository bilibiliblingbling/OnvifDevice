#include "OnvifDeviceControl.h"
#include "OnvifDeviceControlImpl.h"
#include "OnvifDeviceHelper.h"

#include "../include/tmp_log.hpp"
#include "../../../../share/LocalAddressSelector.h"

#include <StreamSockPool.h>
#include <swift/thread/thread.hpp>

#define PREVIEW_STREAM_ID_BASE	64

OnvifDeviceControl::OnvifDeviceControl(OnvifDevicePtr &onvifDevice, DLPCreateClassContextPtr &pCreateClassContext)
    : DLPDeviceControlEx2(pCreateClassContext)
    , m_p_(new OnvifDeviceControlImpl(OnvifDeviceControlPtr(this, true), onvifDevice, pCreateClassContext))
    , m_underRunner(m_engine, 1)
{
}

OnvifDeviceControl::~OnvifDeviceControl()
{
}

DP_RESULT OnvifDeviceControl::Disconnect()
{
    swift::mutex::lock_guard lock(m_lock);

    return DeviceLayer::DLR_S_OK;
}

DP_RESULT OnvifDeviceControl::Configure(const DeviceLayer::ConfigParams &params, DeviceLayer::ConfigParams &failures, IDLPContext *pContext)
{
    swift::mutex::lock_guard lock(m_lock);
    // create a isconnect func in pdevice
    OnvifDeviceHelper::PostHandler(swift::bind(&OnvifDeviceControlImpl::DoConfigure, m_p_, params, pContext));
    return DeviceLayer::DLR_S_OK;
}

DP_RESULT OnvifDeviceControl::OpenStream(const STREAM_ID &id, STREAM_TRANS_PROTO proto, IDLPContext *pContext)
{
    if (proto == STREAM_TRANS_PROTO_ANY)
        proto = STREAM_TRANS_PROTO_UDP;
    swift::mutex::lock_guard lock(m_lock);

    OnvifDeviceHelper::PostHandler(swift::bind(&OnvifDeviceControlImpl::DoOpenStream, m_p_, id, pContext));
    return DeviceLayer::DLR_S_OK;
}

DP_RESULT OnvifDeviceControl::StartMulticast(const STREAM_ID &id, unsigned long multAddr, unsigned short destPort, STREAM_TRANS_PROTO proto, IDLPContext *pContext)
{
    if (proto != STREAM_TRANS_PROTO_ANY && proto != STREAM_TRANS_PROTO_MULTICAST)
        return DeviceLayer::DLR_E_NOT_SUPPORTED;
    swift::mutex::lock_guard lock(m_lock);

    return m_p_->StartMulticast(id, multAddr, destPort, proto, pContext);
}

DP_RESULT OnvifDeviceControl::CloseStream(const DLPStreamExPtr &pStream, IDLPContext *pContext)
{
    swift::mutex::lock_guard lock(m_lock);
    if (pStream->GetTransProto() == STREAM_TRANS_PROTO_MULTICAST) {
        return m_p_->CloseMulticastStream(pStream, pContext);
    } else {
        OnvifDeviceHelper::PostHandler(swift::bind(&OnvifDeviceControlImpl::DoCloseStream, m_p_, pStream, pContext));
    }
    return DeviceLayer::DLR_S_OK;
}

DP_RESULT OnvifDeviceControl::StopMulticast(const STREAM_ID &id, IDLPContext *pContext)
{
    swift::mutex::lock_guard lock(m_lock);
    return m_p_->StopMulticast(id, pContext);
}

DP_RESULT OnvifDeviceControl::Control(int channIndex, DeviceLayer::PTZ_ACTION action, const DeviceLayer::PtzActionParams &params)
{
    ONVIF_DEVICE_UNUSED(channIndex);
    ONVIF_DEVICE_UNUSED(params);

    if (!m_underRunner.is_running())
        m_underRunner.start(1);

    OnvifDeviceHelper::LocalPostHandler(m_engine, swift::bind(&OnvifDeviceControlImpl::DoControl, m_p_, action));

    return DeviceLayer::DLR_S_OK;
}
DP_RESULT OnvifDeviceControl::RequestControl(int channIndex, IDLPContext *pContext)
{

}

DP_RESULT OnvifDeviceControl::ReleaseControl(int channIndex, IDLPContext *pContext)
{

}

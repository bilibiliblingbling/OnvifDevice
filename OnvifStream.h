#ifndef __ONVIF_DEVICE_PLUGIN_STREAM_H__
#define __ONVIF_DEVICE_PLUGIN_STREAM_H__
#include "../../include/ObjectPtr.h"
#include "../../include/ObjectInterfaceDef.h"
#include "../Utils/include/ObjectUtils.h"

#include <swift/thread/mutex.hpp>
#include <swift/net/rtp/rtp.hpp>

#define MAIN_STREAM_ID 1
#define SUB_STREAM_ID 65

using namespace swift::net::rtp;

typedef DLPStreamEx OnvifStream;
typedef DLPStreamExPtr OnvifStreamPtr;

class OnvifInputStream : public DLPInputStreamEx
{
public:
    OnvifInputStream(const STREAM_ID& id, const DLPStreamsExPtr& pStreams, STREAM_TRANS_PROTO proto);
    ~OnvifInputStream();

    // IObject interface
public:
    int AddRef();
    int Release();

    // DLPStreamEx interface
public:
    virtual void StopStream();
    virtual void OnOpened();
    virtual void OnClosed();

public:
    bool IsMainStream();
    bool IsOpened();
    void OnStreamData(const char* data ,int len);
    void OnStreamOpenFailed();
private:
    mutable swift::mutex m_lock;

    bool m_bMainStream;
    bool m_bOpened;
};

typedef DeviceLayer::ObjectPtr<OnvifInputStream> OnvifInputStreamPtr;

#endif ///__ONVIF_DEVICE_PLUGIN_STREAM_H__

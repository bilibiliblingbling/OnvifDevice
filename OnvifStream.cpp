#include "OnvifStream.h"
#include "../../include/DeviceDef.h"

OnvifInputStream::OnvifInputStream(const STREAM_ID &id, const DLPStreamsExPtr &pStreams, STREAM_TRANS_PROTO proto)
    : DLPInputStreamEx(id, pStreams, proto)
    , m_bOpened(false)
    , m_bMainStream(id == MAIN_STREAM_ID ? true : false)
{

}

OnvifInputStream::~OnvifInputStream()
{
}

void OnvifInputStream::StopStream()
{
    //OnClosed();
}

void OnvifInputStream::OnOpened()
{
    swift::mutex::lock_guard lock(m_lock);
    m_bOpened = true;
}

void OnvifInputStream::OnClosed()
{
    swift::mutex::lock_guard lock(m_lock);
    m_bOpened = false;
}

bool OnvifInputStream::IsMainStream()
{
    return m_bMainStream;
}

bool OnvifInputStream::IsOpened()
{
    return m_bOpened;
}

void OnvifInputStream::OnStreamData(const char *data, int len)
{
    DLPInputStreamEx::OnReceiveStreamData(data, len);
}

void OnvifInputStream::OnStreamOpenFailed()
{

}

int OnvifInputStream::AddRef()
{
    return DLPInputStreamEx::AddRef();
}

int OnvifInputStream::Release()
{
    return DLPInputStreamEx::Release();
}

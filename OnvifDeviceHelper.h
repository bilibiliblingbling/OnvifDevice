#ifndef ONVIF_DEVICE_HELPER_H
#define ONVIF_DEVICE_HELPER_H

#include <ObjectPublic.h>
#include <ObjectUtils.h>

#include <swift/function/bind.hpp>
#include <swift/thread/engine.hpp>

// unused
#define ONVIF_DEVICE_UNUSED(x)  (void)(x);

using namespace std;

struct SoapAuthentication
{
    SoapAuthentication(const string &username, const string &password, const string &signaturerole)
    {
        m_username = username;
        m_password = password;
        m_signatureRole = signaturerole;
    }

    SoapAuthentication(const SoapAuthentication &auth)
    {
        m_username = auth.m_username;
        m_password = auth.m_password;
        m_signatureRole = auth.m_signatureRole;
    }
    string m_username;
    string m_password;
    string m_signatureRole;
};

namespace OnvifDeviceHelper
{
// private pool
inline void LocalPostHandler(swift::engine &engine, swift::function0<void> handler)
{
    engine.post(handler);
}

// share pool
inline void PostHandler(swift::function0<void> handler)
{
    DeviceLayer::EngineInstance::Instance().GetEngine().post(handler);
}
} // namespace OnvifDeviceHelper


#endif // ONVIF_DEVICE_HELPER_H

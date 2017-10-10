#include "OnvifDiscovery.h"

OnvifDiscovery::OnvifDiscovery(const string &usrname, const string &password)
    : OnvifBasic(usrname, password)
{
}

OnvifDiscovery::~OnvifDiscovery()
{
}

int OnvifDiscovery::RemoteDiscovery()
{
    if (RemoteDetectDevice(PROBE_MULTICAST_ADDRESS)) {
        if (m_probeResp->wsdd__ProbeMatches->__sizeProbeMatch >=1) {
            int i = 0;
            for(i; i< m_probeResp->wsdd__ProbeMatches->__sizeProbeMatch; i++) {
                //m_deviceServerAddrList.push_back(m_probeResp->wsdd__ProbeMatches->ProbeMatch[i].XAddrs);
            }
        }
    }
}

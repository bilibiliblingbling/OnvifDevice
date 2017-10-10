#include "OnvifDeviceClass.h"

DLP_EXPORT DP_RESULT DLPMETHOD CreateDeviceClass(const DEVICE_CLASS_ID& dcid
                                                 , const INTERFACE_ID& iid
                                                 , void** ppInterface
                                                 , IDLPCreateClassContext* pCreateContext)
{
    return DLPHelper::_CreateDeviceClass<OnvifDeviceClass>(dcid, iid, ppInterface, pCreateContext);
}
DLP_EXPORT DP_RESULT DLPMETHOD CanUnloadNow()
{
    return DLPHelper::_CanUnloadNow();
}

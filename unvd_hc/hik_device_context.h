#ifndef _HIK_DEVICE_CONTEXT_
#define _HIK_DEVICE_CONTEXT_

#include <HCNetSDK.h>
#include <stdint.h>

typedef struct _device_context
{
    LONG HikLoginHandle;
    size_t AnalogCamerasCount;
    uint32_t AnalogCameraStartChannel;
    NET_DVR_COMPRESSIONCFG_ABILITY* pHikCompressioncfgAbilityArray;
    size_t IPCamerasCountConnt;
    uint32_t IPCameraStartChannel;
    NET_DVR_USER_LOGIN_INFO HikLoginInfo;
    NET_DVR_DEVICEINFO_V40 HikLoginDevInfo;
} device_context;

#endif

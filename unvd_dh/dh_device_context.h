#ifndef _DH_DEVICE_CONTEXT_
#define _DH_DEVICE_CONTEXT_

#include <dhnetsdk.h>
#include <dhconfigsdk.h>
#include <stdint.h>

typedef struct _device_context
{
    size_t DHLoginHandle;
    size_t CamerasCount;
    size_t AnalogCamerasCount;
    uint32_t AnalogCameraStartChannel;
    DHDEV_SNAP_CFG* pDevSnapCfg;
    CFG_SNAPCAPINFO_INFO* pCfgSnapCapInfo;
    size_t IPCamerasCountConnt;
    uint32_t IPCameraStartChannel;
    size_t bJSON;
} device_context;

#endif

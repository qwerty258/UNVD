#ifndef _DH_DEVICE_CONTEXT_
#define _DH_DEVICE_CONTEXT_

#include <dhnetsdk.h>
#include <stdint.h>

typedef struct _device_context
{
    size_t DHLoginHandle;
    size_t AnalogCamerasCount;
    uint32_t AnalogCameraStartChannel;

    size_t IPCamerasCountConnt;
    uint32_t IPCameraStartChannel;
    NET_DEVICEINFO DHLoginDevInfo;
} device_context;

#endif

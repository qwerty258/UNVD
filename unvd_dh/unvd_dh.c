#include "unvd_dh.h"
#include "dh_device_context.h"
#include <string.h>
#include <stdlib.h>

UNVDAPI bool UNVD_Initialize()
{
    if (!CLIENT_Init(NULL, 0))
    {
        return false;
    }
    return true;
}

UNVDAPI bool UNVD_CleanUp()
{
    CLIENT_Cleanup();
    return true;
}

UNVDAPI bool UNVD_Login(
    IN_PARAM UNVD_LoginInfo* pUNVD_LoginInfo,
    OUT_PARAM UNVD_LoginHandle* pLoginHandle)
{
    *pLoginHandle = NULL;
    if (NULL == pUNVD_LoginInfo)
        return false;
    device_context* pDeviceContext = calloc(1, sizeof(device_context));
    if (NULL == pDeviceContext)
        return false;

    int errorCode;
    pDeviceContext->DHLoginHandle = CLIENT_Login(
        pUNVD_LoginInfo->szDeviceAddress,
        pUNVD_LoginInfo->port,
        pUNVD_LoginInfo->szUsername,
        pUNVD_LoginInfo->szPassword,
        &pDeviceContext->DHLoginDevInfo,
        &errorCode);
    if (0 == pDeviceContext->DHLoginHandle)
    {
        free(pDeviceContext);
        return false;
    }

    NET_DEVICEINFO* pDeviceInfo = &pDeviceContext->DHLoginDevInfo;
    pDeviceContext->AnalogCamerasCount = pDeviceInfo->byChanNum;
    pDeviceContext->IPCamerasCountConnt = 0;
    pDeviceContext->AnalogCameraStartChannel = 0;
    pDeviceContext->IPCameraStartChannel = 0;

    *pLoginHandle = pDeviceContext;
    return true;
}

UNVDAPI bool UNVD_Logout(IN_PARAM UNVD_LoginHandle* pLoginHandle)
{
    if (NULL == pLoginHandle)
        return false;
    if (NULL == *pLoginHandle)
        return false;
    device_context* pDeviceContext = *pLoginHandle;
    BOOL ret = CLIENT_Logout(pDeviceContext->DHLoginHandle);
    if (ret)
    {
        free(pDeviceContext);
        *pLoginHandle = NULL;
        return true;
    }
    else
    {
        return false;
    }
}

UNVDAPI bool UNVD_GetVideoSourceCount(
    IN_PARAM UNVD_LoginHandle LoginHandle,
    OUT_PARAM size_t* pCount)
{
    if (NULL == LoginHandle)
    {
        return false;
    }
    device_context* pDeviceContext = LoginHandle;
    *pCount = pDeviceContext->AnalogCamerasCount + pDeviceContext->IPCamerasCountConnt;

    return true;
}

UNVDAPI bool UNVD_GetSnapshotData(
    IN_PARAM UNVD_LoginHandle LoginHandle,
    IN_PARAM size_t VideoSourceIndex,
    OUT_PARAM uint8_t* pJEPGBuffer,
    IN_PARAM size_t BufferSize,
    OUT_PARAM size_t* pSizeWritten)
{
    if (NULL == LoginHandle || NULL == pJEPGBuffer || NULL == pSizeWritten)
    {
        return false;
    }
    device_context* pDeviceContext = LoginHandle;

    return false;
}


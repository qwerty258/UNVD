#include "unvd_dh.h"
#include "dh_device_context.h"
#include <string.h>
#include <stdlib.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

UNVDAPI bool UNVD_Initialize()
{
    if (!CLIENT_Init(NULL, 0))
    {
        return false;
    }
    CLIENT_SetConnectTime(5000, 0);

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

    NET_DEVICEINFO DHLoginDevInfo;
    int errorCode;
    pDeviceContext->DHLoginHandle = CLIENT_Login(
        pUNVD_LoginInfo->szDeviceAddress,
        pUNVD_LoginInfo->port,
        pUNVD_LoginInfo->szUsername,
        pUNVD_LoginInfo->szPassword,
        &DHLoginDevInfo,
        &errorCode);
    if (0 == pDeviceContext->DHLoginHandle)
    {
        free(pDeviceContext);
        return false;
    }

    pDeviceContext->CamerasCount = DHLoginDevInfo.byChanNum;
    pDeviceContext->AnalogCamerasCount = 0;
    pDeviceContext->IPCamerasCountConnt = 0;
    pDeviceContext->AnalogCameraStartChannel = 0;
    pDeviceContext->IPCameraStartChannel = 0;

    NET_DEV_CHN_COUNT_INFO DHDevChnCountInfo;

    BOOL ret = CLIENT_QueryDevState(
        pDeviceContext->DHLoginHandle,
        DH_DEVSTATE_DEV_CHN_COUNT,
        (char*)&DHDevChnCountInfo,
        sizeof(NET_DEV_CHN_COUNT_INFO),
        &errorCode,
        1000);
    if (ret)
    {
        pDeviceContext->CamerasCount = MAX(DHDevChnCountInfo.stuVideoIn.nMaxTotal, DHLoginDevInfo.byChanNum);
    }

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
        free(pDeviceContext->pCfgSnapCapInfo);
        free(pDeviceContext->pDevSnapCfg);
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
    *pCount = pDeviceContext->CamerasCount;

    return true;
}

UNVDAPI bool UNVD_GetSnapshotData(
    IN_PARAM UNVD_LoginHandle LoginHandle,
    IN_PARAM size_t VideoSourceIndex,
    OUT_PARAM uint8_t* pJEPGBuffer,
    IN_PARAM size_t BufferSize,
    OUT_PARAM size_t* pSizeWritten)
{
    *pSizeWritten = 0;

    if (NULL == LoginHandle || NULL == pJEPGBuffer || NULL == pSizeWritten)
    {
        return false;
    }
    device_context* pDeviceContext = LoginHandle;

    NET_IN_SNAP_PIC_TO_FILE_PARAM pInParam = { 0 };
    NET_OUT_SNAP_PIC_TO_FILE_PARAM pOutParam = { 0 };

    pInParam.dwSize = sizeof(NET_IN_SNAP_PIC_TO_FILE_PARAM);
    pInParam.stuParam.Channel = VideoSourceIndex;
    pInParam.stuParam.mode = 0;
    pInParam.stuParam.Quality = 6;

    pOutParam.dwPicBufLen = BufferSize;
    pOutParam.dwSize = sizeof(NET_OUT_SNAP_PIC_TO_FILE_PARAM);
    pOutParam.szPicBuf = pJEPGBuffer;

    BOOL ret = CLIENT_SnapPictureToFile(
        pDeviceContext->DHLoginHandle,
        &pInParam,
        &pOutParam,
        1000);
    if (ret)
    {
        *pSizeWritten = pOutParam.dwPicBufRetLen;
    }

    if (ret && *pSizeWritten != 0)
    {
        return true;
    }

    return false;
}


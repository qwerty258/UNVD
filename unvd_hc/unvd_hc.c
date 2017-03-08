#include "unvd_hc.h"
#include "hik_device_context.h"
#include <string.h>
#include <stdlib.h>

typedef struct _PicSizeIndexSizeTable
{
    uint8_t PicSizeIndex;
    size_t PicSize;
} PicSizeIndexSizeTable;

PicSizeIndexSizeTable StaticPicSizeIndexSizeTable[] =
{
    { 0, 352 * 288 },
    { 1, 176 * 144 },
    { 2, 720 * 576 },
    { 3, 1600 * 1200 },
    { 4, 800 * 600 },
    { 5, 1280 * 720 },
    { 6, 640 * 480 },
    { 7, 1280 * 960 },
    { 8, 1600 * 900 },
    { 9, 1920 * 1080 },
    { 10, 2560 * 1920 },
    { 11, 1600 * 304 },
    { 12, 2048 * 1536 },
    { 13, 2448 * 2048 },
    { 14, 2448 * 1200 },
    { 15, 2448 * 800 },
    { 16, 1024 * 768 },
    { 17, 1280 * 1024 },
    { 18, 960 * 576 },
    { 19, 1920 * 1080 },
    { 20, 576 * 576 },
    { 21, 1536 * 1536 },
    { 22, 1920 * 1920 }
};

#define StaticPicSizeIndexSizeTableMax 22

UNVDAPI bool UNVD_Initialize()
{
    if (!NET_DVR_Init())
    {
        return false;
    }
    if (!NET_DVR_SetConnectTime(3000, 3))
    {
        return false;
    }
    if (!NET_DVR_SetRecvTimeOut(5000))
    {
        return false;
    }
    if (!NET_DVR_SetReconnect(30000, false))
    {
        return false;
    }
    return true;
}

UNVDAPI bool UNVD_CleanUp()
{
    return NET_DVR_Cleanup();
}

UNVDAPI bool UNVD_Login(
    IN_PARAM UNVD_LoginInfo* pUNVD_LoginInfo,
    OUT_PARAM UNVD_LoginHandle* pLoginHandle)
{
    *pLoginHandle = NULL;
    if (NULL == pUNVD_LoginInfo)
        return false;
    if (NET_DVR_DEV_ADDRESS_MAX_LEN < strlen(pUNVD_LoginInfo->szDeviceAddress))
        return false;
    if (NET_DVR_LOGIN_USERNAME_MAX_LEN < strlen(pUNVD_LoginInfo->szUsername))
        return false;
    if (NET_DVR_LOGIN_PASSWD_MAX_LEN < strlen(pUNVD_LoginInfo->szPassword))
        return false;
    device_context* pDeviceContext = calloc(1, sizeof(device_context));
    if (NULL == pDeviceContext)
        return false;

    strcpy(
        pDeviceContext->HikLoginInfo.sDeviceAddress,
        pUNVD_LoginInfo->szDeviceAddress);

    pDeviceContext->HikLoginInfo.wPort = pUNVD_LoginInfo->port;

    strcpy(
        pDeviceContext->HikLoginInfo.sUserName,
        pUNVD_LoginInfo->szUsername);

    strcpy(
        pDeviceContext->HikLoginInfo.sPassword,
        pUNVD_LoginInfo->szPassword);

    pDeviceContext->HikLoginInfo.cbLoginResult = NULL;
    pDeviceContext->HikLoginInfo.pUser = NULL;
    pDeviceContext->HikLoginInfo.bUseAsynLogin = false;

    pDeviceContext->HikLoginHandle = NET_DVR_Login_V40(
        &pDeviceContext->HikLoginInfo,
        &pDeviceContext->HikLoginDevInfo);
    if (-1 == pDeviceContext->HikLoginHandle)
    {
        free(pDeviceContext);
        return false;
    }

    NET_DVR_DEVICEINFO_V40* pDeviceInfoV40 = &pDeviceContext->HikLoginDevInfo;
    pDeviceContext->AnalogCamerasCount = pDeviceInfoV40->struDeviceV30.byChanNum;
    pDeviceContext->IPCamerasCountConnt = pDeviceInfoV40->struDeviceV30.byIPChanNum + 256 * pDeviceInfoV40->struDeviceV30.byHighDChanNum;
    pDeviceContext->AnalogCameraStartChannel = pDeviceInfoV40->struDeviceV30.byStartChan;
    pDeviceContext->IPCameraStartChannel = pDeviceInfoV40->struDeviceV30.byStartDChan;

    if (pDeviceContext->AnalogCamerasCount > 0)
    {
        pDeviceContext->pHikCompressioncfgAbilityArray = malloc(pDeviceContext->AnalogCamerasCount * sizeof(NET_DVR_COMPRESSIONCFG_ABILITY));
        if (NULL == pDeviceContext->pHikCompressioncfgAbilityArray)
        {
            UNVD_Logout(&pDeviceContext);
            return false;
        }
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
    BOOL ret = NET_DVR_Logout(pDeviceContext->HikLoginHandle);
    if (ret)
    {
        free(pDeviceContext->pHikCompressioncfgAbilityArray);
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

    // get real index
    uint32_t RealVideoSourceIndex;

    if (VideoSourceIndex < pDeviceContext->AnalogCamerasCount)
    {
        RealVideoSourceIndex = pDeviceContext->AnalogCameraStartChannel + VideoSourceIndex;
    }
    else if (pDeviceContext->AnalogCamerasCount <= VideoSourceIndex &&
        VideoSourceIndex < pDeviceContext->IPCamerasCountConnt + pDeviceContext->AnalogCamerasCount)
    {
        RealVideoSourceIndex = pDeviceContext->IPCameraStartChannel + VideoSourceIndex - pDeviceContext->AnalogCamerasCount;
    }
    else
    {
        return false;
    }

    NET_DVR_JPEGPARA JPEGPara = { 0 };

    JPEGPara.wPicQuality = 0;
    JPEGPara.wPicSize = 0xFF;

    DWORD SizeReturned;

    BOOL ret = NET_DVR_CaptureJPEGPicture_NEW(
        pDeviceContext->HikLoginHandle,
        RealVideoSourceIndex,
        &JPEGPara,
        pJEPGBuffer,
        BufferSize,
        &SizeReturned);
    if (ret)
    {
        if (SizeReturned <= BufferSize)
        {
            *pSizeWritten = SizeReturned;
            return true;
        }
    }
    // if auto size failed, for maximum compatibility, use CIF (0)
    else
    {
        JPEGPara.wPicSize = 0;
        ret = NET_DVR_CaptureJPEGPicture_NEW(
            pDeviceContext->HikLoginHandle,
            RealVideoSourceIndex,
            &JPEGPara,
            pJEPGBuffer,
            BufferSize,
            &SizeReturned);
        if (ret)
        {
            if (SizeReturned <= BufferSize)
            {
                *pSizeWritten = SizeReturned;
                return true;
            }
        }
    }

    *pSizeWritten = 0;
    return false;
}


#include "unvd_dh.h"
#include "dh_device_context.h"
#include <string.h>
#include <stdlib.h>

#if (defined _WIN32 || defined _WIN64)
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#else
#include <sys/param.h>
#endif


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

    DH_DEV_ENABLE_INFO DHDevEnableInfo;

    ret = CLIENT_QuerySystemInfo(
        pDeviceContext->DHLoginHandle,
        ABILITY_DEVALL_INFO,
        (char*)&DHDevEnableInfo,
        sizeof(DH_DEV_ENABLE_INFO),
        &errorCode,
        1000);
    if (ret)
    {
        pDeviceContext->bJSON = DHDevEnableInfo.IsFucEnable[EN_JSON_CONFIG] & 0x01;
    }
    else
    {
        pDeviceContext->bJSON = false;
    }

    if (pDeviceContext->CamerasCount > 0)
    {
        if (pDeviceContext->bJSON)
        {
            pDeviceContext->pCfgSnapCapInfo = malloc(pDeviceContext->CamerasCount * sizeof(CFG_SNAPCAPINFO_INFO));
            if (NULL == pDeviceContext->pCfgSnapCapInfo)
            {
                UNVD_Logout(&pDeviceContext);
                return false;
            }
        }
        else
        {
            pDeviceContext->pDevSnapCfg = malloc(pDeviceContext->CamerasCount * sizeof(DHDEV_SNAP_CFG));
            if (NULL == pDeviceContext->pDevSnapCfg)
            {
                UNVD_Logout(&pDeviceContext);
                return false;
            }
        }
    }

    if (pDeviceContext->bJSON)
    {
        uint32_t JSONBufferSize = 100 * 1024;
        char* pJSONBuffer = malloc(JSONBufferSize);
        if (pJSONBuffer)
        {
            for (size_t i = 0; i < pDeviceContext->CamerasCount; i++)
            {
                ret = CLIENT_GetNewDevConfig(
                    pDeviceContext->DHLoginHandle,
                    CFG_CMD_SNAPCAPINFO,
                    i,
                    pJSONBuffer,
                    JSONBufferSize,
                    &errorCode,
                    500);
                if (!ret)
                {
                    pDeviceContext->pCfgSnapCapInfo[i].nChannelID = -1;
                }
                else
                {
                    ret = CLIENT_ParseData(
                        CFG_CMD_SNAPCAPINFO,
                        pJSONBuffer,
                        &pDeviceContext->pCfgSnapCapInfo[i],
                        sizeof(CFG_SNAPCAPINFO_INFO),
                        NULL);
                    if (!ret)
                    {
                        pDeviceContext->pCfgSnapCapInfo[i].nChannelID = -1;
                    }
                }
            }
            free(pJSONBuffer);
        }
        else
        {
            for (size_t i = 0; i < pDeviceContext->CamerasCount; i++)
            {
                pDeviceContext->pCfgSnapCapInfo[i].nChannelID = -1;
            }
        }
    }
    else
    {
        for (size_t i = 0; i < pDeviceContext->CamerasCount; i++)
        {
            ret = CLIENT_GetDevConfig(
                pDeviceContext->DHLoginHandle,
                DH_DEV_SNAP_CFG,
                i,
                &pDeviceContext->pDevSnapCfg[i],
                sizeof(DHDEV_SNAP_CFG),
                &errorCode,
                500);
            if (!ret)
            {
                pDeviceContext->pDevSnapCfg[i].dwSize = 0;
            }
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
    if (NULL == LoginHandle || NULL == pJEPGBuffer || NULL == pSizeWritten)
    {
        return false;
    }
    device_context* pDeviceContext = LoginHandle;

    return false;
}


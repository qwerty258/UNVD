#include "unvd_dh.h"
#include "dh_device_context.h"
#include <string.h>
#include <stdlib.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#if (defined _WIN32 || defined _WIN64)
#include "fake_pthread_for_win.h"
#else
#include <pthread.h>
#include <time.h>
#endif

pthread_mutex_t PThreadMutexForSnapJobLinkList;
typedef struct _SnapJobLinkListElement SnapJobLinkListElement;
typedef struct _SnapJobLinkListElement
{
    SnapJobLinkListElement* pNext;
    pthread_cond_t PThreadCond;
    uint8_t* pJEPGBuffer;
    size_t BufferSize;
    size_t* pSizeWritten;
    DWORD CmdSerial;
} SnapJobLinkListElement;
SnapJobLinkListElement* pSnapJobLinkListHead = NULL;
DWORD gSnapJobSerial = 0;

void AddSnapJobToLinkList(pthread_cond_t PThreadCond, uint8_t* pJEPGBuffer, size_t BufferSize, size_t* pSizeWritten, DWORD* CmdSerial)
{
    SnapJobLinkListElement* pTemp = malloc(sizeof(SnapJobLinkListElement));
    if (NULL != pTemp)
    {
        pTemp->pNext = NULL;
        pTemp->PThreadCond = PThreadCond;
        pTemp->pJEPGBuffer = pJEPGBuffer;
        pTemp->BufferSize = BufferSize;
        pTemp->pSizeWritten = pSizeWritten;

        pthread_mutex_lock(&PThreadMutexForSnapJobLinkList);

        pTemp->CmdSerial = gSnapJobSerial;
        *CmdSerial = gSnapJobSerial;
        gSnapJobSerial++;

        if (NULL == pSnapJobLinkListHead)
        {
            pSnapJobLinkListHead = pTemp;
        }
        else
        {
            SnapJobLinkListElement* pCur = pSnapJobLinkListHead;
            while (NULL != pCur->pNext)
            {
                pCur = pCur->pNext;
            }
            pCur->pNext = pTemp;
        }

        pthread_mutex_unlock(&PThreadMutexForSnapJobLinkList);
    }
}

void RemoveSnapJobFromLinkList(DWORD CmdSerial)
{
    SnapJobLinkListElement* pPrevious;
    SnapJobLinkListElement* pCurrent;
    SnapJobLinkListElement* pAfter;

    pthread_mutex_lock(&PThreadMutexForSnapJobLinkList);

    if (NULL != pSnapJobLinkListHead)
    {
        pPrevious = pSnapJobLinkListHead;
        pCurrent = pSnapJobLinkListHead;
        pAfter = pSnapJobLinkListHead->pNext;

        while (pCurrent->CmdSerial != CmdSerial && NULL != pCurrent)
        {
            if (pCurrent != pSnapJobLinkListHead)
            {
                pPrevious = pPrevious->pNext;
            }
            pCurrent = pCurrent->pNext;
            if (NULL != pAfter)
            {
                pAfter = pAfter->pNext;
            }
        }

        if (NULL != pCurrent)
        {
            free(pCurrent);
        }

        if (pPrevious != pCurrent)
        {
            pPrevious->pNext = pAfter;
        }
    }

    pthread_mutex_unlock(&PThreadMutexForSnapJobLinkList);
}

void CALLBACK SnapRev(
    LLONG lLoginID,
    BYTE *pBuf,
    UINT RevLen,
    UINT EncodeType,
    DWORD CmdSerial,
    LDWORD dwUser)
{
    SnapJobLinkListElement* pCurrent;

    pthread_mutex_lock(&PThreadMutexForSnapJobLinkList);

    pCurrent = pSnapJobLinkListHead;

    while (pCurrent->CmdSerial != CmdSerial && NULL != pCurrent)
    {
        pCurrent = pCurrent->pNext;
    }

    if (NULL != pCurrent)
    {
        if (pCurrent->BufferSize > RevLen)
        {
            memcpy(pCurrent->pJEPGBuffer, pBuf, RevLen);
            *pCurrent->pSizeWritten = RevLen;
        }
        else
        {
            *pCurrent->pSizeWritten = 0;
        }
    }

    pthread_cond_signal(&pCurrent->PThreadCond);
    pthread_mutex_unlock(&PThreadMutexForSnapJobLinkList);
}

//typedef struct _PicSizeIndexSizeTable
//{
//    uint8_t PicSizeIndex;
//    size_t PicSize;
//} PicSizeIndexSizeTable;
//
//PicSizeIndexSizeTable StaticPicSizeIndexSizeTable[] =
//{
//    { IMAGE_SIZE_D1, 704 * 480 },
//    { IMAGE_SIZE_HD1, 352 * 576 },
//    { IMAGE_SIZE_BCIF, 704 * 288 },
//    { IMAGE_SIZE_CIF, 352 * 288 },
//    { IMAGE_SIZE_QCIF, 176 * 144 },
//    { IMAGE_SIZE_VGA, 640 * 480 },
//    { IMAGE_SIZE_QVGA, 320 * 240 },
//    { IMAGE_SIZE_SVCD, 480 * 480 },
//    { IMAGE_SIZE_QQVGA, 160 * 128 },
//    { IMAGE_SIZE_SVGA, 800 * 592 },
//    { IMAGE_SIZE_XVGA, 1024 * 768 },
//    { IMAGE_SIZE_WXGA, 1280 * 800 },
//    { IMAGE_SIZE_SXGA, 1280 * 1024 },
//    { IMAGE_SIZE_WSXGA, 1600 * 1024 },
//    { IMAGE_SIZE_UXGA, 1600 * 1200 },
//    { IMAGE_SIZE_WUXGA, 1920 * 1200 },
//    { IMAGE_SIZE_LTF, 240 * 192 },
//    { IMAGE_SIZE_720, 1280 * 720 },
//    { IMAGE_SIZE_1080, 1920 * 1080 },
//    { IMAGE_SIZE_1_3M, 1280 * 960 },
//    { IMAGE_SIZE_2M, 1872 * 1408 },
//    { IMAGE_SIZE_5M, 3744 * 1408 },
//    { IMAGE_SIZE_3M, 2048 * 1536 },
//    { IMAGE_SIZE_5_0M, 2432 * 2050 },
//    { IMAGE_SIZE_1_2M, 1216 * 1024 },
//    { IMAGE_SIZE_1408_1024, 1408 * 1024 },
//    { IMAGE_SIZE_8M, 3296 * 2472 },
//    { IMAGE_SIZE_2560_1920, 2560 * 1920 },
//    { IMAGE_SIZE_960H, 960 * 576 },
//    { IMAGE_SIZE_960_720, 960 * 720 },
//    { IMAGE_SIZE_NHD, 640 * 360 },
//    { IMAGE_SIZE_QNHD, 320 * 180 },
//    { IMAGE_SIZE_QQNHD, 160 * 90 },
//};

UNVDAPI bool UNVD_Initialize()
{
    if (!CLIENT_Init(NULL, 0))
    {
        return false;
    }
    CLIENT_SetSnapRevCallBack(SnapRev, 0);

    pthread_mutex_init(&PThreadMutexForSnapJobLinkList, NULL);

    return true;
}

UNVDAPI bool UNVD_CleanUp()
{
    CLIENT_Cleanup();
    pthread_mutex_destroy(&PThreadMutexForSnapJobLinkList);
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

    //DH_DEV_ENABLE_INFO DHDevEnableInfo;

    //ret = CLIENT_QuerySystemInfo(
    //    pDeviceContext->DHLoginHandle,
    //    ABILITY_DEVALL_INFO,
    //    (char*)&DHDevEnableInfo,
    //    sizeof(DH_DEV_ENABLE_INFO),
    //    &errorCode,
    //    1000);
    //if (ret)
    //{
    //    pDeviceContext->bJSON = DHDevEnableInfo.IsFucEnable[EN_JSON_CONFIG] & 0x01;
    //}
    //else
    //{
    //    pDeviceContext->bJSON = false;
    //}

    //if (pDeviceContext->CamerasCount > 0)
    //{
    //    if (pDeviceContext->bJSON)
    //    {
    //        pDeviceContext->pCfgSnapCapInfo = malloc(pDeviceContext->CamerasCount * sizeof(CFG_SNAPCAPINFO_INFO));
    //        if (NULL == pDeviceContext->pCfgSnapCapInfo)
    //        {
    //            UNVD_Logout(&pDeviceContext);
    //            return false;
    //        }
    //    }
    //    else
    //    {
    //        pDeviceContext->pDevSnapCfg = malloc(pDeviceContext->CamerasCount * sizeof(DHDEV_SNAP_CFG));
    //        if (NULL == pDeviceContext->pDevSnapCfg)
    //        {
    //            UNVD_Logout(&pDeviceContext);
    //            return false;
    //        }
    //    }
    //}

    //if (pDeviceContext->bJSON)
    //{
    //    uint32_t JSONBufferSize = 100 * 1024;
    //    char* pJSONBuffer = malloc(JSONBufferSize);
    //    if (pJSONBuffer)
    //    {
    //        for (size_t i = 0; i < pDeviceContext->CamerasCount; i++)
    //        {
    //            ret = CLIENT_GetNewDevConfig(
    //                pDeviceContext->DHLoginHandle,
    //                CFG_CMD_SNAPCAPINFO,
    //                i,
    //                pJSONBuffer,
    //                JSONBufferSize,
    //                &errorCode,
    //                500);
    //            if (!ret)
    //            {
    //                pDeviceContext->pCfgSnapCapInfo[i].nChannelID = -1;
    //            }
    //            else
    //            {
    //                ret = CLIENT_ParseData(
    //                    CFG_CMD_SNAPCAPINFO,
    //                    pJSONBuffer,
    //                    &pDeviceContext->pCfgSnapCapInfo[i],
    //                    sizeof(CFG_SNAPCAPINFO_INFO),
    //                    NULL);
    //                if (!ret)
    //                {
    //                    pDeviceContext->pCfgSnapCapInfo[i].nChannelID = -1;
    //                }
    //            }
    //        }
    //        free(pJSONBuffer);
    //    }
    //    else
    //    {
    //        for (size_t i = 0; i < pDeviceContext->CamerasCount; i++)
    //        {
    //            pDeviceContext->pCfgSnapCapInfo[i].nChannelID = -1;
    //        }
    //    }
    //}
    //else
    //{
    //    for (size_t i = 0; i < pDeviceContext->CamerasCount; i++)
    //    {
    //        ret = CLIENT_GetDevConfig(
    //            pDeviceContext->DHLoginHandle,
    //            DH_DEV_SNAP_CFG,
    //            i,
    //            &pDeviceContext->pDevSnapCfg[i],
    //            sizeof(DHDEV_SNAP_CFG),
    //            &errorCode,
    //            500);
    //        if (!ret)
    //        {
    //            pDeviceContext->pDevSnapCfg[i].dwSize = 0;
    //        }
    //    }
    //}

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

    SNAP_PARAMS SnapParams;

    SnapParams.Channel = VideoSourceIndex;

    // get best quality

    //uint32_t BestQuality;

    //if (pDeviceContext->bJSON)
    //{
    //    CFG_SNAPCAPINFO_INFO* pCfgSnapCapInfo = &pDeviceContext->pCfgSnapCapInfo[VideoSourceIndex];
    //    if (-1 != pCfgSnapCapInfo->nChannelID)
    //    {
    //        BestQuality = 1;
    //        for (DWORD i = 0; i < pCfgSnapCapInfo->dwQualityMun; i++)
    //        {
    //            if (pCfgSnapCapInfo->emQualityList[i] > BestQuality)
    //            {
    //                BestQuality = pCfgSnapCapInfo->emQualityList[i];
    //            }
    //        }
    //    }
    //    else
    //    {
    //        BestQuality = 6;
    //    }
    //}
    //else
    //{
    //    DHDEV_SNAP_CFG* pDevSnapCfg = &pDeviceContext->pDevSnapCfg[VideoSourceIndex];
    //    if (1 == pDevSnapCfg->struSnapEnc[0].byImageQltyType)
    //    {
    //        BestQuality = 6;
    //    }
    //    else
    //    {
    //        BestQuality = pDevSnapCfg->struSnapEnc[0].byImageQlty;
    //    }
    //}

    //SnapParams.Quality = BestQuality;

    SnapParams.Quality = 6;
    // get largest pic

    //uint32_t LargestSize;

    //if (pDeviceContext->bJSON)
    //{
    //    CFG_SNAPCAPINFO_INFO* pCfgSnapCapInfo = &pDeviceContext->pCfgSnapCapInfo[VideoSourceIndex];
    //    if (-1 != pCfgSnapCapInfo->nChannelID)
    //    {
    //        LargestSize = 0;
    //        for (DWORD i = 0; i < pCfgSnapCapInfo->dwIMageSizeNum; i++)
    //        {
    //            if (pCfgSnapCapInfo->emIMageSizeList[i] >= IMAGE_SIZE_NR)
    //            {
    //                continue;
    //            }
    //            if (StaticPicSizeIndexSizeTable[pCfgSnapCapInfo->emIMageSizeList[i]].PicSize>StaticPicSizeIndexSizeTable[LargestSize].PicSize)
    //            {
    //                LargestSize = pCfgSnapCapInfo->emIMageSizeList[i];
    //            }
    //        }
    //    }
    //    else
    //    {
    //        LargestSize = 0;
    //    }
    //}
    //else
    //{
    //    LargestSize = 2;
    //}

    //SnapParams.ImageSize = LargestSize;

    SnapParams.ImageSize = 0;
    SnapParams.mode = 0;

    pthread_cond_t PThreadCond;
    if (0 != pthread_cond_init(&PThreadCond, NULL))
    {
        return false;
    }
    AddSnapJobToLinkList(PThreadCond, pJEPGBuffer, BufferSize, pSizeWritten, &SnapParams.CmdSerial);

    BOOL ret = CLIENT_SnapPictureEx(
        pDeviceContext->DHLoginHandle,
        &SnapParams,
        NULL);
    if (ret)
    {
        pthread_mutex_t temp;
        struct timespec weakuptime;

#if (defined _WIN32 || defined _WIN64)
        weakuptime.tv_nsec = 0;
        weakuptime.tv_sec = 1;
#else
        pthread_mutex_init(&temp, NULL);
        pthread_mutex_lock(&temp);
        struct timespec weakuptime;
        timespec_get(&weakuptime, TIME_UTC);
        weakuptime.tv_sec += 1;
#endif
        pthread_cond_timedwait(&PThreadCond, &temp, &weakuptime);

#if (defined _WIN32 || defined _WIN64)
#else
        pthread_mutex_unlock(&temp);
        pthread_mutex_destroy(&temp);
#endif
    }

    RemoveSnapJobFromLinkList(SnapParams.CmdSerial);
    pthread_cond_destroy(&PThreadCond);

    if (ret && *pSizeWritten != 0)
    {
        return true;
    }

    return false;
}


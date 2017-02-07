#ifndef _UNVD_HC_H_
#define _UNVD_HC_H_

#include <unvddef.h>
#include <stddef.h>
#include <stdbool.h>

UNVDAPI bool UNVD_Initialize(void);

UNVDAPI bool UNVD_CleanUp(void);

UNVDAPI bool UNVD_Login(
    IN_PARAM UNVD_LoginInfo* pUNVD_LoginInfo,
    OUT_PARAM UNVD_LoginHandle* pLoginHandle);

UNVDAPI bool UNVD_Logout(IN_PARAM UNVD_LoginHandle* pLoginHandle);

UNVDAPI bool UNVD_GetVideoSourceCount(
    IN_PARAM UNVD_LoginHandle LoginHandle,
    OUT_PARAM size_t* pCount);

UNVDAPI bool UNVD_GetSnapshot(
    IN_PARAM UNVD_LoginHandle LoginHandle,
    IN_PARAM size_t Channel,
    OUT_PARAM uint8_t* pJEPGBuffer,
    IN_PARAM size_t BufferSize,
    OUT_PARAM size_t* pSizeWritten);

#endif

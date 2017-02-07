#ifndef INCLUDE_UNVDDEF_H_
#define INCLUDE_UNVDDEF_H_

#ifdef __cplusplus
  #ifdef UNVD_DLL_EXPORT
    #define UNVDAPI extern "C" __declspec(dllexport)
  #else
    #define UNVDAPI extern "C"
  #endif
#else
  #ifdef UNVD_DLL_EXPORT
    #define UNVDAPI __declspec(dllexport)
  #else
    #define UNVDAPI
  #endif
#endif

#define IN_PARAM
#define OUT_PARAM

#include <stdint.h>

typedef void* UNVD_LoginHandle;
typedef struct _UNVD_LoginInfo
{
    char* szDeviceAddress;
    uint16_t port;
    char* szUsername;
    char* szPassword;
} UNVD_LoginInfo;

#endif /* INCLUDE_UNVDDEF_H_ */

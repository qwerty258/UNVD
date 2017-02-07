#include <stdio.h>
#include <stdlib.h>
#include <unvd_hc.h>
int main(int argc, char* argv[])
{
    bool ret;
    ret = UNVD_Initialize();

    UNVD_LoginInfo LoginInfo;
    UNVD_LoginHandle LoginHandle;

    LoginInfo.szDeviceAddress = "192.168.2.208";
    LoginInfo.port = 8000;
    LoginInfo.szUsername = "admin";
    LoginInfo.szPassword = "12345";

    if (UNVD_Login(&LoginInfo, &LoginHandle))
    {
        printf("connect success\n");

        size_t VideoSourceCount;
        UNVD_GetVideoSourceCount(LoginHandle, &VideoSourceCount);

        uint8_t* pBuffer = malloc(1920 * 1080 * 3 / 2);
        size_t BytesWritten;
        bool ret = UNVD_GetSnapshotData(
            LoginHandle,
            0,
            pBuffer,
            1920 * 1080 * 3 / 2,
            &BytesWritten);

        if (ret)
        {
#if (defined _WIN32 || defined _WIN64)
            FILE* pFile = fopen("D:\\test.jpg", "wb");
#else
            FILE* pFile = fopen("/home/qwe/test.jpg", "wb");
#endif
            fwrite(pBuffer, BytesWritten, 1, pFile);
            fclose(pFile);
        }
        else
        {
            printf("capture failed\n");
        }

        UNVD_Logout(&LoginHandle);
        free(pBuffer);
    }
    else
    {
        printf("connect failed\n");
    }

    ret = UNVD_CleanUp();
    return 0;
}

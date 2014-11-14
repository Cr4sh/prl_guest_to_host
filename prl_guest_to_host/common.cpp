#include "stdafx.h"
//--------------------------------------------------------------------------------------
BOOL GetResPayload(HMODULE hModule, char *lpszResourceName, PVOID *Data, DWORD *dwDataSize)
{
    HRSRC hRc = FindResource(hModule, lpszResourceName, BINRES_NAME);
    if (hRc)
    {
        HGLOBAL hResData = LoadResource(hModule, hRc);
        if (hResData)
        {
            PVOID ResData = LockResource(hResData);
            if (ResData)
            {
                *dwDataSize = SizeofResource(hModule, hRc);
                if (*Data = M_ALLOC(*dwDataSize))
                {
                    memcpy(*Data, ResData, *dwDataSize);
                    return TRUE;
                }
                else
                {
                    DbgMsg(__FILE__, __LINE__, "M_ALLOC() ERROR %d\r\n", GetLastError());
                }
            }
            else
            {
                DbgMsg(__FILE__, __LINE__, "LockResource() fails\r\n");
            }
        }
        else
        {
            DbgMsg(__FILE__, __LINE__, "LoadResource() fails\r\n");
        }
    }
    else
    {
        DbgMsg(__FILE__, __LINE__, "FindResource() fails\r\n");
    }

    return FALSE;
}
//--------------------------------------------------------------------------------------
BOOL ReadFromFile(LPCTSTR lpszFileName, PVOID *pData, PDWORD lpdwDataSize)
{
    BOOL bRet = FALSE;
    HANDLE hFile = CreateFile(
        lpszFileName, 
        GENERIC_READ, 
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 
        NULL,
        OPEN_EXISTING, 
        0, 
        NULL
    );
    if (hFile != INVALID_HANDLE_VALUE)
    {
        if (pData == NULL || lpdwDataSize == NULL)
        {
            // just check for existing file
            bRet = TRUE;
            goto close;
        }

        *lpdwDataSize = GetFileSize(hFile, NULL);
        if (*pData = M_ALLOC(*lpdwDataSize))
        {
            DWORD dwReaded = 0;
            if (ReadFile(hFile, *pData, *lpdwDataSize, &dwReaded, NULL))
            {
                bRet = TRUE;
            }            
            else
            {
                DbgMsg(__FILE__, __LINE__, "ReadFile() ERROR %d\n", GetLastError());
            }
        }
        else
        {
            DbgMsg(__FILE__, __LINE__, "M_ALLOC() ERROR %d\n", GetLastError());
            *lpdwDataSize = 0;
        }

close:
        CloseHandle(hFile);
    }
    else
    {
        DbgMsg(__FILE__, __LINE__, "CreateFile() ERROR %d\n", GetLastError());
    }

    return bRet;
}
//--------------------------------------------------------------------------------------
BOOL DumpToFile(LPCTSTR lpszFileName, PVOID pData, ULONG DataSize)
{
    BOOL bRet = FALSE;
    HANDLE hFile = CreateFile(lpszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD dwWritten = 0;
        if (WriteFile(hFile, pData, DataSize, &dwWritten, NULL))
        {
            bRet = TRUE;
        }
        else
        {
            DbgMsg(__FILE__, __LINE__, "WriteFile() ERROR %d\n", GetLastError());
        }

        CloseHandle(hFile);
    }
    else
    {
        DbgMsg(__FILE__, __LINE__, "CreateFile() ERROR %d\n", GetLastError());
    }

    return bRet;
}
//--------------------------------------------------------------------------------------
// EoF

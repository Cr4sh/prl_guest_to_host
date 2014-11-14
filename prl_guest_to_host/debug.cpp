#include "stdafx.h"
//--------------------------------------------------------------------------------------
#ifdef DBG
//--------------------------------------------------------------------------------------
char *GetNameFromFullPath(char *lpszPath)
{
    char *lpszName = lpszPath;

    for (size_t i = 0; i < strlen(lpszPath); i++)
    {
        if (lpszPath[i] == '\\' || lpszPath[i] == '/')
        {
            lpszName = lpszPath + i + 1;
        }
    }

    return lpszName;
}
//--------------------------------------------------------------------------------------
void DbgMsg(char *lpszFile, int Line, char *lpszMsg, ...)
{
    va_list mylist;
    va_start(mylist, lpszMsg);

    size_t len = _vscprintf(lpszMsg, mylist) + 0x100;

    char *lpszBuff = (char *)LocalAlloc(LMEM_FIXED, len);
    if (lpszBuff == NULL)
    {
        va_end(mylist);
        return;
    }

    char *lpszOutBuff = (char *)LocalAlloc(LMEM_FIXED, len);
    if (lpszOutBuff == NULL)
    {
        LocalFree(lpszBuff);
        va_end(mylist);
        return;
    }

    vsprintf_s(lpszBuff, len, lpszMsg, mylist);	
    va_end(mylist);

    sprintf_s(
        lpszOutBuff, len, "[%.5d] .\\%s(%d) : %s", 
        GetCurrentProcessId(), GetNameFromFullPath(lpszFile), Line, lpszBuff
    );

    OutputDebugStringA(lpszOutBuff);

    HANDLE hStd = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStd != INVALID_HANDLE_VALUE)
    {
        DWORD dwWritten = 0;
        WriteFile(hStd, lpszBuff, lstrlenA(lpszBuff), &dwWritten, NULL);    
    }

    LocalFree(lpszOutBuff);
    LocalFree(lpszBuff);
}
//--------------------------------------------------------------------------------------
#endif DBG
//--------------------------------------------------------------------------------------
// EoF

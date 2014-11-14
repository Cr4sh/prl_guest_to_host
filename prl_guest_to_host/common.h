
#define TIME_ABSOLUTE(wait) (wait)
#define TIME_RELATIVE(wait) (-(wait))
#define TIME_NANOSECONDS(nanos) (((signed __int64)(nanos)) / 100L)
#define TIME_MICROSECONDS(micros) (((signed __int64)(micros)) * TIME_NANOSECONDS(1000L))
#define TIME_MILLISECONDS(milli) (((signed __int64)(milli)) * TIME_MICROSECONDS(1000L))
#define TIME_SECONDS(seconds) (((signed __int64)(seconds)) * TIME_MILLISECONDS(1000L))

#define RVATOVA(_base_, _offset_) ((PUCHAR)(_base_) + (ULONG)(_offset_))

#define M_ALLOC(_size_) LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, (ULONG)(_size_))
#define M_FREE(_addr_) LocalFree((_addr_))

#define XALIGN_DOWN(x, align) (x &~ (align - 1))
#define XALIGN_UP(x, align) ((x & (align - 1)) ? XALIGN_DOWN(x, align) + align : x)

#define IFMT32 "0x%.8x"
#define IFMT64 "0x%.16I64x"


#define GET_NATIVE(_name_)                                      \
                                                                \
    func_##_name_ f_##_name_ = (func_##_name_)GetProcAddress(   \
        GetModuleHandle(_T("ntdll.dll")),                       \
        (#_name_)                                               \
    );

#define UNICODE_FROM_WCHAR(_us_, _str_)                         \
                                                                \
    ((PUNICODE_STRING)(_us_))->Buffer = (_str_);                \
    ((PUNICODE_STRING)(_us_))->Length =                         \
    ((PUNICODE_STRING)(_us_))->MaximumLength =                  \
    (USHORT)wcslen((_str_)) * sizeof(WCHAR);


#if defined(_X86_)

#define IFMT IFMT32

#elif defined(_AMD64_)

#define IFMT IFMT64

#endif

#define BINRES_NAME "BINRES"

BOOL GetResPayload(HMODULE hModule, char *lpszResourceName, PVOID *Data, DWORD *dwDataSize);

BOOL ReadFromFile(LPCTSTR lpszFileName, PVOID *pData, PDWORD lpdwDataSize);
BOOL DumpToFile(LPCTSTR lpszFileName, PVOID pData, ULONG DataSize);

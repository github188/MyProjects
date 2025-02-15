// SystemEmulation.cpp

#include <Windows.h>

#define WINAPI_DECL     __declspec(dllexport)

#include "SystemEmulation.h"
#include "FileSystemEmulation.h"
#include "Charset.h"

#include <algorithm>
#include <string>

#include <assert.h>
#include <stdio.h>
#include <io.h>

__declspec(dllexport) char ** environ = NULL;

namespace FileSystemEmulation
{

    DWORD WINAPI_DECL GetLocalPathA(
        _In_   DWORD nBufferLength,
        _Out_  LPSTR lpBuffer);

}

namespace SystemEmulation
{

#undef FormatMessageA

    DWORD WINAPI_DECL FormatMessage2A(
        _In_      DWORD dwFlags,
        _In_opt_  LPCVOID lpSource,
        _In_      DWORD dwMessageId,
        _In_      DWORD dwLanguageId,
        _Out_     LPSTR lpBuffer,
        _In_      DWORD nSize,
        _In_opt_  va_list *Arguments
        )
    {
        LPSTR lpBuffer2 = lpBuffer;
        if (dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER) {
            dwFlags &= ~FORMAT_MESSAGE_ALLOCATE_BUFFER;
            lpBuffer2 = (LPSTR)new char [512];
            nSize = 512;
        }
        LPWSTR lpWideCharStr = (LPWSTR)new WCHAR[nSize];
        DWORD dw = ::FormatMessageW(
            dwFlags, 
            lpSource, 
            dwMessageId, 
            dwLanguageId, 
            lpWideCharStr, 
            nSize, 
            Arguments);
        if (dw == 0) {
            delete [] lpBuffer2;
            delete [] lpWideCharStr;
            return 0;
        }
        dw = ::WideCharToMultiByte(CP_ACP, 0, lpWideCharStr, dw + 1, lpBuffer2, nSize, 0, FALSE);
        delete [] lpWideCharStr;
        if (lpBuffer2 != lpBuffer) {
            *(LPSTR *)lpBuffer = lpBuffer2;
        }
        return dw;
    }

    void WINAPI_DECL OutputDebugString2A(
        _In_opt_  LPCSTR lpOutputString
        )
    {
        int cchWideChar = strlen(lpOutputString) + 1;
        LPWSTR lpWideCharStr = (LPWSTR)new WCHAR[cchWideChar];
        if ((cchWideChar = ::MultiByteToWideChar(CP_ACP, 0, lpOutputString, cchWideChar, lpWideCharStr, cchWideChar)) == 0) {
            delete [] lpWideCharStr;
            return;
        }
        OutputDebugStringW(lpWideCharStr);
        delete [] lpWideCharStr;
    }

    DWORD WINAPI_DECL GetTickCount(void)
    {
        return (DWORD)GetTickCount64();
    }

    void WINAPI_DECL GetSystemInfo(
        _Out_  LPSYSTEM_INFO lpSystemInfo
        )
    {
        GetNativeSystemInfo(lpSystemInfo);
    }

    static void init_environ()
    {
        if (environ != NULL)
            return;
        environ = new char *[32];
        memset(environ, 0, sizeof(char *) * 32);
        int n = 0;
        {
        char * TMP = new char[MAX_PATH + 8];
        strncpy_s(TMP, MAX_PATH + 8, "TMP=", 4);
        FileSystemEmulation::GetTempPathA(MAX_PATH, TMP + 4);
        environ[n++] = TMP;
        }
        {
        char * CFG = new char[MAX_PATH + 16];
        strncpy_s(CFG, MAX_PATH + 16, "LD_CONFIG_PATH=", 15);
        FileSystemEmulation::GetLocalPathA(MAX_PATH, CFG + 15);
        environ[n++] = CFG;
        }
    }

    LPCH WINAPI_DECL GetEnvironmentStringsA(void)
    {
        return "";
    }

    DWORD WINAPI_DECL GetEnvironmentVariableA(
        _In_opt_   LPCSTR lpName,
        _Out_opt_  LPSTR lpBuffer,
        _In_       DWORD nSize
        )
    {
        init_environ();
        char const * const * p = environ;
        size_t len = strlen(lpName);
        while (*p) {
            if (strncmp(*p, lpName, len) == 0 && (*p)[len] == '=') {
                size_t len2 = strlen(*p + len + 1);
                if (nSize == 0) {
                    *(LPCSTR *)lpBuffer = *p + len + 1;
                    return len2;
                } else if (nSize < len2 + 1) {
                    return len2 + 1;
                } else {
                    strncpy_s(lpBuffer, nSize, *p + len + 1, len2);
                    return len2;
                }
            }
            ++p;
        }
        SetLastError(ERROR_ENVVAR_NOT_FOUND);
        return 0;
    }

    BOOL WINAPI_DECL SetEnvironmentVariableA(
        _In_      LPCSTR lpName,
        _In_opt_  LPCSTR lpValue
        )
    {
        assert(0);
        return FALSE;
    }

	int _wtol(
        _In_       LPCWSTR lpStr
		)
	{
		return 0;
	}

    LCID WINAPI_DECL GetUserDefaultLCID(void)
    {
        return (LCID)LOCALE_SYSTEM_DEFAULT;
    }

    int WINAPI_DECL LCMapStringA(
        _In_       LCID Locale,
        _In_       DWORD dwMapFlags,
        _In_       LPCSTR lpSrcStr,
        _In_       int cchSrc,
        _Out_opt_  LPSTR lpDestStr,
        _In_       int cchDest
        )
    {
		assert(Locale == LOCALE_SYSTEM_DEFAULT);
        WCHAR wszCodePage[7];
        if (::GetLocaleInfoEx(
            LOCALE_NAME_SYSTEM_DEFAULT, 
            LOCALE_IDEFAULTANSICODEPAGE, 
            wszCodePage, 
            7) == 0) {
                return 0;
        }
        UINT code_page = static_cast<UINT>(_wtol(wszCodePage));
        charset_t charset1(code_page, lpSrcStr, cchSrc);
        if (charset1.wstr() == NULL) {
            return 0;
        }
        charset_t charset2(code_page, lpDestStr, cchDest);
        if (charset2.wstr() == NULL) {
            return 0;
        }
        int cchWideChar2 = LCMapStringW(
            Locale, 
            dwMapFlags, 
            charset1.wstr(), 
            charset1.wlen(), 
            charset2.wstr(), 
            charset2.wlen());
        if (cchWideChar2 == 0) {
            return 0;
        }
        charset2.wlen(cchWideChar2);
        charset2.w2a();
        return charset2.len();
    }

    int WINAPI_DECL LCMapStringW(
        _In_       LCID Locale,
        _In_       DWORD dwMapFlags,
        _In_       LPCWSTR lpSrcStr,
        _In_       int cchSrc,
        _Out_opt_  LPWSTR lpDestStr,
        _In_       int cchDest
        )
    {
		assert(Locale == LOCALE_SYSTEM_DEFAULT);
		if (dwMapFlags == LCMAP_LOWERCASE) {
			LPWSTR out = std::transform(lpSrcStr, lpSrcStr + cchSrc, lpDestStr, towlower);
			return out - lpDestStr;
		} else if (dwMapFlags == LCMAP_SORTKEY)  {
			if (lpDestStr != NULL) {
				memcpy(lpDestStr, lpSrcStr, cchSrc);
			}
			return cchSrc;
		} else {
			assert(false);
		}
    }

    BOOL WINAPI_DECL GetStringTypeExA(
        _In_   LCID Locale,
        _In_   DWORD dwInfoType,
        _In_   LPCSTR lpSrcStr,
        _In_   int cchSrc,
        _Out_  LPWORD lpCharType
        )
    {
		assert(Locale == LOCALE_SYSTEM_DEFAULT);
        WCHAR wszCodePage[7];
        if (::GetLocaleInfoEx(
            LOCALE_NAME_SYSTEM_DEFAULT, 
            LOCALE_IDEFAULTANSICODEPAGE, 
            wszCodePage, 
            7) == 0) {
                return FALSE;
        }
        UINT nCodePage = static_cast<UINT>(_wtol(wszCodePage));
        charset_t charset(nCodePage, lpSrcStr, cchSrc);
        if (charset.wstr() == NULL) {
            return FALSE;
        }
        BOOL b = GetStringTypeExW(
            Locale, 
            dwInfoType, 
            charset.wstr(), 
            charset.wlen(), 
            lpCharType);
        return b;
    }

    int WINAPI_DECL LoadStringA(
        _In_opt_  HINSTANCE hInstance,
        _In_      UINT uID,
        _Out_     LPSTR lpBuffer,
        _In_      int nBufferMax
        )
    {
        assert(false);
        return 0;
    }

    int WINAPI_DECL LoadStringW(
        _In_opt_  HINSTANCE hInstance,
        _In_      UINT uID,
        _Out_     LPWSTR lpBuffer,
        _In_      int nBufferMax
        )
    {
        assert(false);
        return 0;
    }

    HANDLE WINAPI_DECL GetStdHandle(
        _In_  DWORD nStdHandle
        )
    {
        switch (STD_OUTPUT_HANDLE)
        {
        case STD_INPUT_HANDLE:
            return (HANDLE)_get_osfhandle(_fileno(stdin));
        case STD_OUTPUT_HANDLE:
            return (HANDLE)_get_osfhandle(_fileno(stdout));
        case STD_ERROR_HANDLE:
            return (HANDLE)_get_osfhandle(_fileno(stderr));
        default:
            assert(false);
            return NULL;
        }
    }

    BOOL WINAPI_DECL SetConsoleTextAttribute(
        _In_  HANDLE hConsoleOutput,
        _In_  WORD wAttributes
        )
    {
        return FALSE;
    }

    BOOL WINAPI_DECL FileTimeToLocalFileTime(
        _In_   const FILETIME *lpFileTime,
        _Out_  LPFILETIME lpLocalFileTime
        )
    {
        SYSTEMTIME SystemTime;
        SYSTEMTIME LocalSystemTime;
        if (FALSE == FileTimeToSystemTime(
            lpFileTime, 
            &SystemTime))
            return FALSE;
        if (FALSE == SystemTimeToTzSpecificLocalTime(
            NULL, 
            &SystemTime, 
            &LocalSystemTime))
            return FALSE;
        if (FALSE == SystemTimeToFileTime(
            &LocalSystemTime, 
            lpLocalFileTime))
            return FALSE;
        return TRUE;
    }

    HMODULE WINAPI_DECL GetModuleHandleA(
        _In_opt_  LPCSTR lpModuleName
        )
    {
        charset_t charset(lpModuleName);
        if (charset.wstr() == NULL) {
            return (HMODULE)GetCurrentProcess();
        }
        HMODULE hModule = LoadPackagedLibrary(
            charset.wstr(), 
            0);
        return hModule;
    }

    HMODULE WINAPI_DECL LoadLibraryA(
        _In_  LPCSTR lpFileName
        )
    {
        charset_t charset(lpFileName);
        if (charset.wstr() == NULL) {
            return NULL;
        }
        HMODULE hModule = LoadPackagedLibrary(
            charset.wstr(), 
            0);
        return hModule;
    }

    DWORD WINAPI_DECL GetModuleFileNameA(
      _In_opt_  HMODULE hModule,
      _Out_     LPSTR lpFilename,
      _In_      DWORD nSize
    )
    {
		if (hModule != NULL) {
			SetLastError(ERROR_NOT_SUPPORTED);
			return 0;
		}
        Platform::String ^ path = 
            Windows::ApplicationModel::Package::Current->InstalledLocation->Path;
        Platform::String ^ name = 
            //Windows::ApplicationModel::Package::Current->Id->Name;
            ref new Platform::String(L"Daemon.exe");
        DWORD nBufferLength = charset_t::w2a(
            path->Data(), path->Length(), 
            lpFilename, nSize - 1);
        if (nBufferLength == 0)
            return 0;
        if (nBufferLength + 1 < nSize) {
            lpFilename[nBufferLength++] = '\\';
        }
        nBufferLength += charset_t::w2a(
            name->Data(), name->Length(), 
            lpFilename + nBufferLength, nSize - nBufferLength - 1);
        if (nBufferLength < nSize) {
            lpFilename[nBufferLength] = '\0';
        }
        return nBufferLength;
    }

}

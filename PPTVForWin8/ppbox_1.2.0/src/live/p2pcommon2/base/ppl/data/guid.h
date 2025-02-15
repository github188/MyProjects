#ifndef _LIVE_P2PCOMMON2_BASE_PPL_DATA_GUID_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_DATA_GUID_H_

/**
 * @file
 * @brief guid相关的类和函数
 */

#include <ppl/config.h>

#include <ppl/data/tstring.h>
#include <ppl/util/macro.h>
#include <ppl/util/log.h>

#include <assert.h>
#include <stdio.h>

#if defined(_PPL_PLATFORM_MSWIN)
#include <objbase.h>
#pragma comment(lib, "ole32.lib")
#else // defined(_PPL_PLATFORM_MSWIN)
#if !defined(_PPL_WITH_LIB_UUID)
#include <ppl/data/uuid.h>
#else // !defined(_PPL_WITH_LIB_UUID)
#include <uuid/uuid.h>
#endif // !defined(_PPL_WITH_LIB_UUID)

typedef struct _GUID {
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[ 8 ];
} GUID;

extern const GUID GUID_NULL;

#define PPL_DEFINE_GUID_NULL extern const GUID GUID_NULL = { 0 }

inline bool operator==(const GUID& guidOne, const GUID& guidOther)
{
    return 0 == memcmp(&guidOne, &guidOther, sizeof(GUID));
}

inline int operator!=(const GUID& guidOne, const GUID& guidOther)
{
    return !(guidOne == guidOther);
}

#endif // defined(_PPL_PLATFORM_MSWIN)

/// 格式化GUID
bool FormatGUID(char* result, size_t size, const GUID& guid);

/// 格式化GUID
bool FormatGUID(wchar_t* result, size_t size, const GUID& guid);

/// 将GUID格式化为字符串
tstring FormatGUID(const GUID& guid);

/// 解析guid字符串
bool ParseGUID(GUID& guid, const string& guidstr);
#ifdef _GLIBCXX_USE_WCHAR_T
bool ParseGUID(GUID& guid, const wstring& guidstr);
#endif

/// 解析guid字符串
bool ParseGUID(GUID& guid, const char* guidstr);
#ifdef _GLIBCXX_USE_WCHAR_T
bool ParseGUID(GUID& guid, const wchar_t* guidstr);
#endif

/// 将GUID输出到流
inline std::ostream& operator<<(std::ostream& os, const GUID& guid)
{
	return os << FormatGUID(guid);
}

/// 封装GUID相关的操作
class Guid : public GUID
{
public:
	Guid()
	{
		Clear();
	}
	Guid(const GUID& guid) : GUID(guid)
	{
	}

	void operator=(const GUID& guid)
	{
		GUID& me = *this;
		me = guid;
	}

	/// 清空
	void Clear()
	{
		FILL_ZERO(*this);
	}

	explicit Guid(const TCHAR* guidstr)
	{
		Parse(guidstr);
	}
	explicit Guid(const tstring& guidstr)
	{
		Parse(guidstr);
	}
	/// 解析字符串得到GUID
	bool Parse(const char* guidstr) { return Parse(string(guidstr)); }
#ifdef _GLIBCXX_USE_WCHAR_T
	bool Parse(const wchar_t* guidstr) { return Parse(wstring(guidstr)); }
#endif

	/// 解析字符串得到GUID
	bool Parse(const string& guidstr) { return ParseGUID(*this, guidstr); }
#ifdef _GLIBCXX_USE_WCHAR_T
	bool Parse(const wstring& guidstr) { return ParseGUID(*this, guidstr); }
#endif

	/// 随机生成一个(唯一的)GUID
	void Generate()
	{
		GenerateGUID(*this);
	}

	/// 转换为字符串
	tstring ToString() const { return FormatGUID(*this); }

	static void GenerateGUID(GUID& guid)
	{
#if defined(_PPL_PLATFORM_MSWIN)
		HRESULT hr = ::CoCreateGuid(&guid);
		LIVE_ASSERT(S_OK == hr);
#elif defined(_PPL_PLATFORM_LINUX)
		::uuid_generate(reinterpret_cast<unsigned char*>(&guid));
#else
#error no guid generator
#endif

	}
};

/// 自动随机生成的guid
class RandomGuid : public Guid
{
public:
	RandomGuid()
	{
		Generate();
	}
};

inline bool FormatGUID(char* result, size_t size, const GUID& guid)
{
	int len = snprintf(
				result, 
				size, 
				"{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", 
				guid.Data1, guid.Data2, guid.Data3, 
				guid.Data4[0], guid.Data4[1], 
				guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]
				);
	if (len == sizeof(GUID) * 2 + 6)
		return true;
	UTIL_ERROR("FormatGUID failed. len=" << len);
	LIVE_ASSERT(!"FormatGUID failed.");
	return false;
}


inline tstring FormatGUID(const GUID& guid)
{
	const size_t max_size = 128;
	TCHAR result[max_size + 1] = { 0 };
	FormatGUID(result, max_size, guid);
	return result;
}

inline string FormatAnsiGUID(const GUID& guid)
{
	const size_t max_size = 128;
	char result[max_size + 1] = { 0 };
	FormatGUID(result, max_size, guid);
	return result;
}

#if defined(_PPL_PLATFORM_MSWIN)

inline bool FormatGUID(wchar_t* result, size_t size, const GUID& guid)
{
	int len = _snwprintf(
				result, 
				size, 
				L"{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", 
				guid.Data1, guid.Data2, guid.Data3, 
				guid.Data4[0], guid.Data4[1], 
				guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]
				);
	if (len == sizeof(GUID) * 2 + 6)
		return true;
	UTIL_ERROR("FormatGUID failed. len=" << len);
	LIVE_ASSERT(!"FormatGUID failed.");
	return false;
}

inline wstring FormatWideGUID(const GUID& guid)
{
	const size_t max_size = 128;
	WCHAR result[max_size + 1] = { 0 };
	FormatGUID(result, max_size, guid);
	return result;
}

inline bool DoParseGUID(GUID& guid, const wchar_t* guidstr)
{
	LIVE_ASSERT(guidstr != NULL);
	const int guid_fields = 11;

	unsigned int fields0 = 0;		 
	unsigned int fields[2];
	unsigned int fields2[8];

	int count = swscanf(
		guidstr, 
		L"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", 
		&(fields0), &(fields[0]), &(fields[1]), 
		&(fields2[0]), &(fields2[1]), 
		&(fields2[2]), &(fields2[3]), 
		&(fields2[4]), &(fields2[5]), 
		&(fields2[6]), &(fields2[7])
		);
	if (guid_fields != count)
		return false;
	guid.Data1 = fields0;
	guid.Data2 = static_cast<unsigned short>(fields[0]);
	guid.Data3 = static_cast<unsigned short>(fields[1]);
	for (int i = 0; i < 8; ++i)
	{
		guid.Data4[i] = static_cast<unsigned char>(fields2[i]);
	}
	return true;
}

#endif // defined(_PPL_PLATFORM_MSWIN)

inline bool DoParseGUID(GUID& guid, const char* guidstr)
{
	LIVE_ASSERT(guidstr != NULL);
	const int guid_fields = 11;

	unsigned int fields0 = 0;		 
	unsigned int fields[2];
	unsigned int fields2[8];

	int count = sscanf(
		guidstr, 
		"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", 
		&(fields0), &(fields[0]), &(fields[1]), 
		&(fields2[0]), &(fields2[1]), 
		&(fields2[2]), &(fields2[3]), 
		&(fields2[4]), &(fields2[5]), 
		&(fields2[6]), &(fields2[7])
		);
	if (guid_fields != count)
		return false;
	guid.Data1 = fields0;
	guid.Data2 = static_cast<unsigned short>(fields[0]);
	guid.Data3 = static_cast<unsigned short>(fields[1]);
	for (int i = 0; i < 8; ++i)
	{
		guid.Data4[i] = static_cast<unsigned char>(fields2[i]);
	}
	return true;
}

inline bool ParseGUID(GUID& guid, const char* guidstr)
{
	return ParseGUID(guid, string(guidstr));
}

inline bool ParseGUID(GUID& guid, const string& guidstr)
{
	string str = guidstr;
	if (str.empty())
		return false;
	if (str[0] == '{')
	{
		str.erase(0, 1);
	}
	if (str[str.size() - 1] == '}')
	{
		str.erase(str.size() - 1, 1);
	}
	bool res = DoParseGUID(guid, str.c_str());
	//LIVE_ASSERT(res);
	return res;
}

#ifdef _GLIBCXX_USE_WCHAR_T
inline bool ParseGUID(GUID& guid, const wchar_t* guidstr)
{
	return ParseGUID(guid, wstring(guidstr));
}

inline bool ParseGUID(GUID& guid, const wstring& guidstr)
{
	wstring str = guidstr;
	if (str.empty())
		return false;
	if (str[0] == L'{')
	{
		str.erase(0, 1);
	}
	if (str[str.size() - 1] == L'}')
	{
		str.erase(str.size() - 1, 1);
	}
	bool res = DoParseGUID(guid, (const char*)str.c_str());
	//LIVE_ASSERT(res);
	return res;
}
#endif

inline bool operator<( const Guid& x, const Guid& y )
{
	const GUID& gx = x;
	const GUID& gy = y;
	return memcmp(&gx, &gy, sizeof(GUID)) < 0;
}

#endif // PPL_GUID_DEFINED

#ifdef _PPL_RUN_TEST

#include <ppl/data/strings.h>
#include <ppl/util/test_case.h>

class GuidTestCase : public ppl::util::test_case
{
public:
	GuidTestCase() : m_guidstr("{12345cc9-1234-46fc-b092-ba8820058955}"), m_guidstr2("12345cc9-1234-46fc-b092-ba8820058955")
	{
	}

private:
	const string m_guidstr;
	const string m_guidstr2;

	void DoRun()
	{
		TestFunction();
		TestClass();
	}

	void TestFunction()
	{
		GUID guid;
		LIVE_ASSERT(ParseGUID(guid, m_guidstr.c_str()));
		tstring guidstr2 = FormatGUID(guid);
		tstring guidstrUpper = strings::upper(guidstr2);
		LIVE_ASSERT(guidstr2 == guidstrUpper);

		LIVE_ASSERT(ParseGUID(guid, m_guidstr2.c_str()));
		tstring guidstr3 = FormatGUID(guid);
		LIVE_ASSERT(guidstr3 == guidstrUpper);
	}

	void TestClass()
	{
		Guid guid;
		LIVE_ASSERT(guid.Parse(m_guidstr.c_str()));
		tstring guidstr2 = guid.ToString();
		tstring guidstrUpper = strings::upper(guidstr2);
		LIVE_ASSERT(guidstr2 == guidstrUpper);

		LIVE_ASSERT(guid.Parse(m_guidstr2.c_str()));
		tstring guidstr3 = guid.ToString();
		LIVE_ASSERT(guidstr3 == guidstrUpper);
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(GuidTestCase);

#endif


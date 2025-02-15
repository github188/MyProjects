
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_OS_FILE_SYSTEM_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_OS_FILE_SYSTEM_H_

#include <ppl/config.h>

#include <ppl/util/macro.h>
#include <ppl/data/tchar.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <boost/algorithm/string/replace.hpp>
#include <boost/static_assert.hpp>



#if defined(_PPL_PLATFORM_MSWIN)

#include <direct.h>

#include <shlobj.h>
#include <shellapi.h>
#pragma comment(lib, "shell32.lib")

namespace ppl_detail {

inline bool api_copy_file( const TCHAR* srcFilePath, const TCHAR* destFilePath )
{
	BOOL success = ::CopyFile(srcFilePath, destFilePath, FALSE);
	LIVE_ASSERT(success);
	return FALSE != success;
}

inline bool api_create_directory( const TCHAR* dir )
{
	bool success = (FALSE != ::CreateDirectory( dir, NULL ));
	LIVE_ASSERT( success );
	return success;
}

inline tstring api_get_temp_dir()
{
	const size_t max_path_size = 1023;
	TCHAR buf[max_path_size + 1];
	buf[max_path_size] = '\0';
	DWORD len = ::GetTempPath( max_path_size, buf );
	return tstring( buf, len );
}

inline bool api_remove_all( const TCHAR* path )
{
	SHFILEOPSTRUCT ops;
	FILL_ZERO( ops );
	ops.wFunc = FO_DELETE;
	ops.pFrom = path;
	ops.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
	return 0 == ::SHFileOperation( &ops );
}

inline bool api_create_directories( const TCHAR* dir )
{
	tstring dirstr(dir);
	boost::replace_all( dirstr, "/", "\\" );
	SECURITY_ATTRIBUTES sa;
	FILL_ZERO(sa);
	sa.nLength = sizeof(sa);
	return 0 == ::SHCreateDirectoryEx( NULL, dirstr.c_str(), &sa );
}

}


#else

#if defined(_PPL_PLATFORM_MSWIN)
#include <unistd.h>
#endif

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>

namespace ppl_detail {

inline bool api_copy_file( const char* srcFilePath, const char* destFilePath )
{
	boost::system::error_code err = boost::filesystem::detail::copy_file_api( srcFilePath, destFilePath );
#ifdef _DEBUG
	if ( err )
	{
		ostringstream sout;
		sout << err;
		printf("failed to invoke FileInfo::CopyTo %s %s %s\n", srcFilePath, destFilePath, sout.str().c_str());
		LIVE_ASSERT(false);
		}
#endif
	return !err;
}

inline bool api_create_directory( const char* dir )
{
	bool success = (0 == mkdir( dir, 0770 ));
	LIVE_ASSERT( success );
	return success;
}

#define _trename rename
#define _tunlink unlink
#define _trmdir rmdir

inline string api_get_temp_dir()
{
	return "/tmp";
}

inline bool api_create_directories( const char* dir )
{
	boost::filesystem::path p(dir);
	return boost::filesystem::create_directories( p );
}

inline bool api_remove_all( const TCHAR* path )
{
	boost::filesystem::path p(path);
	return boost::filesystem::remove_all( p ) > 0;
}

}

#endif


namespace ppl { namespace os {


class file_status
{
public:
#ifdef _PPL_PLATFORM_MSWIN
	typedef struct _stat stat_struct;
	static int get_stat( const char* filename, stat_struct* data )
	{
		return _stat( filename, data );
	}
#else
	typedef struct stat stat_struct;
	static int get_stat( const char* filename, stat_struct* data )
	{
		return stat( filename, data );
	}
#endif

	file_status()
	{
		FILL_ZERO(m_data);
	}

	bool retrieve(const char* filename)
	{
		int res = get_stat(filename, &m_data);
		return ( 0 == res );
	}
	bool retrieve(const string& filename)
	{
		return this->retrieve(filename.c_str());
	}

#ifdef _PPL_PLATFORM_MSWIN
	bool retrieve(const wchar_t* filename)
	{;
		int res = _wstat( filename, &m_data );
		return ( 0 == res );
	}
	bool retrieve(const wstring& filename)
	{
		return this->retrieve(filename.c_str());
	}
#endif

	const stat_struct& data() const { return m_data; }
	stat_struct& data() { return m_data; }

	const stat_struct* operator->() const { return &m_data; }
	stat_struct* operator->() { return &m_data; }

	bool is_regular() const
	{
		return check_mode(S_IFREG);
	}

	bool is_file() const
	{
		return is_regular();
	}

	bool is_directory() const
	{
		return check_mode(S_IFDIR);
	}

	time_t creation_time() const { return m_data.st_ctime; }
	time_t last_modify_time() const { return m_data.st_mtime; }
	time_t last_access_time() const { return m_data.st_atime; }


	bool check_mode(unsigned short modeFlag) const
	{
		return (m_data.st_mode & modeFlag) != 0;
	}


private:
	stat_struct m_data;
};


class file_system
{
public:
	static bool exists( const TCHAR* path )
	{
		file_status s;
		return s.retrieve(path);
	}

	static bool directory_exists( const TCHAR* path )
	{
		file_status s;
		return s.retrieve(path) && s.is_directory();
	}

	static bool file_exists( const TCHAR* path )
	{
		file_status s;
		return s.retrieve(path) && s.is_file();
	}

	static bool ensure_directory_exists( const TCHAR* dir )
	{
		if ( directory_exists( dir ) )
			return true;
		return create_directories( dir );
	}

	static bool create_directory( const TCHAR* path )
	{
		return ppl_detail::api_create_directory( path );
	}


	static bool create_directories( const TCHAR* path )
	{
		return ppl_detail::api_create_directories( path );
	}


	static bool copy_file( const TCHAR* srcFilePath, const TCHAR* destFilePath )
	{
		return ppl_detail::api_copy_file(srcFilePath, destFilePath);
	}

	static bool move( const TCHAR* srcFilePath, const TCHAR* destFilePath )
	{
		return 0 == ::_trename(srcFilePath, destFilePath);
	}

	static bool rename( const TCHAR* srcFilePath, const TCHAR* destFilePath )
	{
		return 0 == ::_trename(srcFilePath, destFilePath);
	}

	static bool remove_file( const TCHAR* path )
	{
		return (0 == ::_tunlink(path));
	}

	static bool remove_empty_directory( const TCHAR* path )
	{
		return ( 0 == ::_trmdir( path ) );
	}

	static tstring get_temp_directory()
	{
		return ppl_detail::api_get_temp_dir();
	}

	static bool remove_all( const TCHAR* path )
	{
		return ppl_detail::api_remove_all( path );
	}
};

} }


#endif

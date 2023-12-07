#include "quickfile.h"
#include "os\win32.h"

enum { QF_ErrorLen = 511 };

static nchar_t g_last_error[QF_ErrorLen + 1] = {0}; 

static inline QF_flags_t _quickf_flags_win32(const uint attribs)
{
	if (attribs == (uint)-1)
		return 0;
#define TO_FLAG(from, to) (attribs & (from) ? (to) : 0)

	return	QF_Exists |
					TO_FLAG(FILE_ATTRIBUTE_HIDDEN, QF_Hidden)    |
					TO_FLAG(FILE_ATTRIBUTE_READONLY, QF_ReadOnly)  |
					TO_FLAG(FILE_ATTRIBUTE_DIRECTORY, QF_Dir) |
					TO_FLAG(FILE_ATTRIBUTE_ENCRYPTED, QF_Encrypted);
	
#undef TO_FLAG
}

QF_flags_t qf_get_flags(const nchar_t *path)
{
#ifdef WIN32

	const DWORD dwAttrib = WIN_API(GetFileAttributes)(path);
	return _quickf_flags_win32(dwAttrib);

#endif // WIN32
}

QF_stats_t qf_get_stats(const nchar_t* path)
{
#ifdef WIN32

	WIN32_FILE_ATTRIBUTE_DATA win32_data = {0};
	QF_stats_t state = {0};

	const BOOL success = WIN_API(GetFileAttributesEx)(path, GetFileExInfoStandard, &win32_data);

	if (!success)
	{
		const DWORD err = GetLastError();

		if (err == ERROR_FILE_NOT_FOUND)
		{
			state.flags = 0;
		}
		else
		{
			nc_sprintf_s(g_last_error, QF_ErrorLen, "Win32 Error ID %d", err);
			state.flags = QF_Error;
		}
		return state;
	}

	state.flags = _quickf_flags_win32(win32_data.dwFileAttributes);
	state.creation_time = ((ulong)win32_data.ftCreationTime.dwHighDateTime << 32) + win32_data.ftCreationTime.dwLowDateTime;
	state.last_access_time = ((ulong)win32_data.ftLastAccessTime.dwHighDateTime << 32) + win32_data.ftLastAccessTime.dwLowDateTime;
	state.last_write_time = ((ulong)win32_data.ftLastWriteTime.dwHighDateTime << 32) + win32_data.ftLastWriteTime.dwLowDateTime;

	// if exists and not a dir, set it's size
	if (state.flags && !(state.flags & QF_Dir))
	{
		state.size = ((ulong)win32_data.nFileSizeHigh << 32) + win32_data.nFileSizeLow;
	}

	return state;
#endif // WIN32
}

const nchar_t* qf_get_error()
{
	return g_last_error;
}

uint16_t qf_filetime_to_unixtime(FileTime_t filetime)
{
	
	return 0;
}

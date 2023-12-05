#include "quickfile.h"
#include <Windows.h>

enum { QF_ErrorLen = 511 };

static nchar_t g_last_error[QF_ErrorLen + 1] = {0}; 

static inline QF_flags_t _quickf_state_win32(const uint attribs)
{
	if (attribs == (uint)-1)
		return QF_NotFound;
#define TO_FLAG(from, to) (attribs & (from) ? (to) : 0)

	return	TO_FLAG(FILE_ATTRIBUTE_HIDDEN, QF_Dir)    |
					TO_FLAG(FILE_ATTRIBUTE_READONLY, QF_Dir)  |
					TO_FLAG(FILE_ATTRIBUTE_DIRECTORY, QF_Dir) |
					TO_FLAG(FILE_ATTRIBUTE_ENCRYPTED, QF_Dir);
	
#undef TO_FLAG
}

QF_flags_t qf_get_flags(const nchar_t *path)
{
#ifdef WIN32

	const DWORD dwAttrib = WIN_API(GetFileAttributes)(path);
	return _quickf_state_win32(dwAttrib);

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
		nc_strcpy_s(g_last_error, QF_ErrorLen, GetLastError());
		state.flags = QF_Error;
		return state;
	}

#endif // WIN32
}

const nchar_t* qf_get_error()
{
	return g_last_error;
}

#pragma once
#include "time.h"

typedef enum
{
	QF_Exists			= 1 << 0,
	QF_Hidden			= 1 << 1,
	QF_ReadOnly		= 1 << 2,
	QF_Dir				= 1 << 3,
	QF_Encrypted	= 1 << 4,
	
	QF_Error			= 1 << 31,

} QF_flags_t;

typedef struct
{
	QF_flags_t flags;
	FileTime_t creation_time;
	FileTime_t last_access_time, last_write_time;
	ulong size;
} QF_stats_t;

extern QF_flags_t qf_get_flags(const nchar_t* path);
extern QF_stats_t qf_get_stats(const nchar_t* path);

// Ptr NOT owned by user
extern const nchar_t *qf_get_error();

// from begin to end
static inline long long fsize(FILE *f)
{
	const long long cur = _ftelli64(f);
	(void)_fseeki64(f, 0, SEEK_END);
	const long long end = _ftelli64(f);
	(void)_fseeki64(f, cur, SEEK_SET);
	return end;
}

// from current to end
static inline long long fend(FILE *f)
{
	const long long cur = _ftelli64(f);
	(void)_fseeki64(f, 0, SEEK_END);
	const long long end = _ftelli64(f);
	(void)_fseeki64(f, cur, SEEK_SET);
	return end - cur;
}


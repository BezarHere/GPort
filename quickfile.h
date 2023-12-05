#pragma once
#include "gh.h"
typedef enum
{
	QF_NotFound		= 1 << 0,
	QF_Hidden			= 1 << 1,
	QF_ReadOnly		= 1 << 2,
	QF_Dir				= 1 << 3,
	QF_Encripted	= 1 << 4,
	
	QF_Error			= 1 << 31,
} QF_flags_t;

typedef struct
{
	QF_flags_t flags;
	ulong creation_time;
	ulong last_access_time;
	ulong last_write_time;
	size_t size;
} QF_stats_t;

extern QF_flags_t qf_get_flags(const nchar_t* path);
extern QF_stats_t qf_get_stats(const nchar_t* path);

// Ptr NOT owned by user
extern const nchar_t *qf_get_error();

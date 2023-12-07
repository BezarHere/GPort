#include "time.h"
// tiletime starts at the 1st of January, 1601 (UTC)
// unixtime starts at the 1st of January, 1970 (UTC)

#define WINDOWS_TICK 10000000
#define SEC_TO_UNIX_EPOCH 11644473600ll

UnixTime_t filetime_to_unixtime(FileTime_t filetime)
{
	return (UnixTime_t)(filetime / WINDOWS_TICK - SEC_TO_UNIX_EPOCH);
}

FileTime_t unixtime_to_filetime(UnixTime_t unixtime)
{
	return ((FileTime_t)unixtime + SEC_TO_UNIX_EPOCH) * WINDOWS_TICK;
}

uint filetime_get_second(FileTime_t filetime)
{
	return (filetime / WINDOWS_TICK) % 60;
}


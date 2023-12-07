#pragma once
#include "gh.h"
typedef long long int FileTime_t;
typedef int UnixTime_t;

extern UnixTime_t filetime_to_unixtime(FileTime_t filetime);
extern FileTime_t unixtime_to_filetime(UnixTime_t unixtime);

extern uint filetime_get_second(FileTime_t filetime); // [0, 60)

#pragma once
#include "gh.h"

typedef struct 
{
	bool isdir;
	bool isencripted;
	bool ishidden;
	size_t filesize;
	uint last_access_time;
	uint last_write_time;
} DirEntryData_t;


typedef struct 
{
	char *path; // [should me freed]
	DirEntryData_t data;
} DirEntry_t;

typedef struct 
{
	wchar_t *path; // [should me freed]
	DirEntryData_t data;
} WDirEntry_t;

extern DirEntry_t *dirscan_search(const char *dirpath, size_t *count);
extern WDirEntry_t *wdirscan_search(const wchar_t *dirpath, size_t *count);

extern void dirscan_free(DirEntry_t *entries, size_t count);
extern void wdirscan_free(WDirEntry_t *entries, size_t count);

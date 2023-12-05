#include "dirscan.h"
#include "os/win32.h"
enum { MaxPathSize = 1<<10, MaxPerDirSearchIterations = 1<<10 };

typedef struct 
{
	wchar_t *path;
	WIN32_FIND_DATAW find_data;
} dir_find_entry_t;

static inline wchar_t *strexpandW(const char *s, const size_t n)
{
	wchar_t *w = malloc(sizeof(wchar_t) * (n + 1));
	ASSERT(w != NULL);
	for (size_t i = 0; i < n; i++)
	{
		w[i] = s[i];
	}
	w[n] = 0;
	return w;
}

static inline char *wcsshrinkdS(const wchar_t *w, const size_t n)
{
	char *s = malloc(sizeof(char) * (n + 1));
	ASSERT(s != NULL);
	for (size_t i = 0; i < n; i++)
	{
		s[i] = w[i] & 0xff;
	}
	s[n] = 0;
	return s;
}

static inline dir_find_entry_t *_scan_dir_win32( size_t depth,
	                                               const wchar_t *dirpath,
                                                 const wchar_t *mask,
																								 dir_find_entry_t *entries,
																								 size_t *pentries_index,
																								 size_t *pentries_len       )
{
	
	if (!depth)
		return entries;
	depth--;

	ASSERT(dirpath != NULL);
	ASSERT(mask != NULL);
	ASSERT(entries != NULL);
	ASSERT(pentries_index != NULL);
	ASSERT(pentries_len != NULL);

	WIN32_FIND_DATAW fdfile;
	HANDLE hfile;

	const size_t dirpath_len = wcslen(dirpath), mask_len = wcslen(mask);
	wchar_t searchterm[MaxPathSize];
	wsprintfW(searchterm, L"%s\\%s", dirpath, mask);

#ifdef VERBOSE
	wprintf(L"scaning dir \"%s\" with mask \"%s\": \"%s\"\n", dirpath, mask, searchterm);
#endif

	if ((hfile = FindFirstFileW(searchterm, &fdfile)) == INVALID_HANDLE_VALUE)
	{
		wprintf(L"ERR: couldn't scan directory \"%s\" masked with \"%s\"\n", dirpath, mask);
		return entries;
	}

	size_t entries_index = *pentries_index;
	size_t entries_len = *pentries_len;

	size_t _iterations_limiter = 0;
	do
	{
		const size_t filename_len = wcsnlen(fdfile.cFileName, 255);
		
		// first entries are L'.' and L'..'
		if ((filename_len <= 2 && fdfile.cFileName[0] == L'.') && (filename_len == 1 || fdfile.cFileName[1] == L'.'))
			continue;
		
		
		entries[entries_index].find_data = fdfile;

		entries[entries_index].path = malloc((dirpath_len + filename_len + 2) * sizeof(wchar_t)); // <- check for funny stuff

		wsprintfW(entries[entries_index].path, L"%s\\%s", dirpath, fdfile.cFileName);
		entries[entries_index].path[dirpath_len + filename_len + 1] = 0;

#ifdef VERBOSE
		wprintf(L"found entry path '%s'\n", entries[entries_index].path);
#endif

		if (++entries_index >= entries_len)
		{
			// bad alloc may happen for large searchs
			if (entries_len > (1 << 14))
				entries_len = (size_t)(entries_len * 1.5);
			else
				entries_len *= 2;

			entries = realloc(entries, sizeof(dir_find_entry_t) * entries_len);
		}

		if (fdfile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			const size_t subdir_filepath_len = dirpath_len + filename_len + 1;

			wchar_t *subdir_path = malloc((subdir_filepath_len + 1) * sizeof(wchar_t));

			wsprintfW(subdir_path, L"%s\\%s", dirpath, fdfile.cFileName);
			subdir_path[subdir_filepath_len] = 0;

			entries = _scan_dir_win32(depth, entries[entries_index - 1].path, mask, entries, &entries_index, &entries_len);

			free(subdir_path);
		}

	} while (FindNextFileW(hfile, &fdfile) && _iterations_limiter++ < MaxPerDirSearchIterations);

	FindClose(hfile);

	*pentries_index = entries_index;
	*pentries_len = entries_len;

	return entries;
}

static inline dir_find_entry_t *scan_dir_win(const wchar_t *dirpath, const wchar_t *mask, _Out_ size_t *count)
{
	ASSERT(count != NULL);

#ifdef VERBOSE
	wprintf(L"starting to scan \"%s\"\n", dirpath);
#endif

	size_t entry_index = 0, entries_len = 64;
	dir_find_entry_t *dir_find_entries = malloc(sizeof(dir_find_entry_t) * entries_len);
	ASSERT(dir_find_entries != NULL);

	dir_find_entries = _scan_dir_win32(256, dirpath, mask, dir_find_entries, &entry_index, &entries_len);

	*count = entry_index;
	return dir_find_entries;
}

static void *return_passed_wchat_t(wchar_t *s)
{
	return s;
}

static void *shrink_free_wchar_t(wchar_t *s)
{
	char *shrinked = wcsshrinkdS(s, wcslen(s));
	free(s);
	return shrinked;
}


static inline DirEntry_t *_dirscan_imp(const dir_find_entry_t *entries, size_t count, void *(p_path_processor)(wchar_t *))
{
	DirEntry_t *out_entries = malloc(sizeof(DirEntry_t) * count);
	ASSERT(out_entries != NULL);

	for (size_t i = 0; i < count; i++)
	{
		out_entries[i].path = p_path_processor(entries[i].path);
		out_entries[i].data.isdir = entries[i].find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
		out_entries[i].data.isencripted = entries[i].find_data.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED;
	}

	return out_entries;
}

DirEntry_t * dirscan_search(const char * dirpath, size_t *pcount)
{
	size_t count = 0;

	wchar_t *expanded = strexpandW(dirpath, strlen(dirpath));

	dir_find_entry_t *scaned_dirs = scan_dir_win(expanded, L"*.*", &count);

	free(expanded);

	DirEntry_t *out_entries = (DirEntry_t *)_dirscan_imp(scaned_dirs, count, shrink_free_wchar_t);

	free(scaned_dirs);
	*pcount = count;
	return out_entries;
}

WDirEntry_t *wdirscan_search(const wchar_t *dirpath, size_t *pcount)
{
	size_t count = 0;

	dir_find_entry_t *scaned_dirs = scan_dir_win(dirpath, L"*.*", &count);

	WDirEntry_t *out_entries = (WDirEntry_t *)_dirscan_imp(scaned_dirs, count, return_passed_wchat_t);

	free(scaned_dirs);
	*pcount = count;
	return out_entries;
}

void dirscan_free(DirEntry_t *entries, size_t count)
{
	for (size_t i = 0; i < count; i++)
	{
		free(entries[i].path);
	}
	free(entries);
}

void wdirscan_free(WDirEntry_t *entries, size_t count)
{
	dirscan_free((DirEntry_t *)entries, count);
}

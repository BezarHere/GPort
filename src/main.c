#include "gh.h"
#include "main.h"
#include "dirscan.h"
#include "path.h"
#include "os/win32.h"

static inline void run_process(nchar_t* args)
{

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	si.hStdOutput = stdout;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

#ifdef UNICHAR_MODE
	CreateProcessW(NULL, args, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
#else
	CreateProcessA(NULL, args, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
#endif

	// Wait until child process exits.
	WaitForSingleObject(pi.hProcess, INFINITE);

	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

}

int main(int argc, char* argv[])
{
	nc_printf(STR("nchar_t size: %zd\n"), sizeof(nchar_t));
	const nchar_t* path = "F:\\GameLog.log";
	QF_stats_t state = qf_get_stats(path);
	if (state.flags & QF_Error)
	{
		nc_printf(STR("QF Error: %s\n"), qf_get_error());
	}
	else
		printf("quickfile flags: %zd\n", state.flags);
	printf("quickfile size: %zd\n", state.size);
	printf("quickfile ct: %zd\n", state.creation_time);

	for (size_t i = 0; i < 1; i++)
	{
		path_t ppath = path_create(path);
		nchar_t* parent = path_parent(&ppath), * filename = path_filename(&ppath), * extn = path_extension(&ppath);
		// nc_printf(STR("parent: \"%s\", filename: '%s', ext: '%s'\n"), parent, filename, extn);
		free(parent);
		free(filename);
		free(extn);
		path_close(&ppath);


		// size_t count;
		// DirEntry_t *entries = dirscan_search(path, &count);
		// dirscan_free(entries, count);
		// printf("ran iteration %d\n", i);

	}
	printf("done iterations!\n");
	char c[32];
	gets_s(c, 1);
}


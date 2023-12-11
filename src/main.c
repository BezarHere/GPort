#include "gh.h"
#include "main.h"
#include "dirscan.h"
#include "path.h"
#include "setting_file.h"
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
	nc_printf(STR("nchar_t size: %lld\n"), sizeof(nchar_t));
	const nchar_t* path = STR("F:\\GameLog.log");
	QF_stats_t state = qf_get_stats(path);
	if (state.flags & QF_Error)
	{
		nc_printf(STR("QF Error: %s\n"), qf_get_error());
	}
	else
		printf("quickfile flags: %d\n", state.flags);
	printf("quickfile size: %lld\n", state.size);
	printf("quickfile ct: %lld\n", state.creation_time);
	FILE *fp = fopen("F:\\Assets\\gcc\\GPort\\test.stgf", "r");

	for (size_t i = 0; i < 1; i++)
	{
		SettingFile_t st = stgf_fread(fp);
		stgf_close(&st);
		fseek(fp, 0, SEEK_SET);

		// path_t ppath = path_create(path);
		// nchar_t* parent = path_parent(&ppath), * filename = path_filename(&ppath), * extn = path_extension(&ppath);
		// // nc_printf(STR("parent: \"%s\", filename: '%s', ext: '%s'\n"), parent, filename, extn);
		// free(parent);
		// free(filename);
		// free(extn);
		// path_close(&ppath);


		// size_t count;
		// DirEntry_t *entries = dirscan_search(path, &count);
		// dirscan_free(entries, count);
		// printf("ran iteration %d\n", i);

	}
	printf("done iterations!\n");
	char c[32];
	gets_s(c, 1);
}


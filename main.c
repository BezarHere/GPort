#include "gh.h"
#include "main.h"
#include "dirscan.h"
#include "path.h"
#include <Windows.h>

int main(int argc, char *argv[])
{
	char_printf(STR("CHAR size: %zd\n"), sizeof(CHAR));
	const CHAR *path = STR("\\\\Assets\\ACLib\\main.png\\");

	for (size_t i = 0; i < 1; i++)
	{
		path_t ppath = path_create(path);
		CHAR *parent = path_parent(&ppath), *filename = path_filename(&ppath), *extn = path_extension(&ppath);
		// char_printf(STR("parent: \"%s\", filename: '%s', ext: '%s'\n"), parent, filename, extn);
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


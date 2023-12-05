#include "gh.h"
#include "main.h"
#include "dirscan.h"
#include "path.h"

int main(int argc, char *argv[])
{
	char_printf(STR("CHAR size: %zd\n"), sizeof(CHAR));
	const CHAR *path = STR("F:\\Assets\\ACLib\\main.png");
	for (size_t i = 0; i < 1; i++)
	{
		path_t ppath = path_create(path);
		char_printf(STR("path: %zd\n"), ppath._segments_count);
		CHAR *c = path_flatten(&ppath);
		char_printf(STR("path: %d %d %d %d %d \"%s\"\n"), ppath._segments[0][0], ppath._segments[0][1], ppath._segments[0][2], ppath._segments[0][3], ppath._segments[0][4], c);
		free(c);
		path_close(&ppath);

		// size_t count;
		// DirEntry_t *entries = dirscan_search(path, &count);
		// dirscan_free(entries, count);
		// printf("ran iteration %d\n", i);
	}
	char c[32];
	gets_s(c, 1);
}


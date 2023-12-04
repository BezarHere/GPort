#include "main.h"
#include "dirscan.h"

int main(int argc, char *argv[])
{
	const char *path = "F:\\Assets\\ACLib";
	for (size_t i = 0; i < 64; i++)
	{
		size_t count;
		DirEntry_t *entries = dirscan_deep(path, &count);
		dirscan_free(entries, count);
		// printf("ran iteration %d\n", i);
	}
}


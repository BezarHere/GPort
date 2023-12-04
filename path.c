#include "path.h"

static inline size_t path_length(const path_t *path)
{
	return 1 + char_strlen_s(path->body, Path_BodyMaxLength) + char_strlen_s(path->head, Path_HeadMaxLength);
}

CHAR *path_flatten(const path_t *path)
{
	const size_t path_len = path_length(path);
	CHAR *str = malloc(sizeof(CHAR) * (path_len + 1));
	
	(void)char_sprintf_s(str, path_len, "%c:\\%s\\%s", path->tail, path->body, path->tail);
	str[path_len] = 0;
	return str;
}

path_t path_create(const CHAR *filepath)
{
	
}
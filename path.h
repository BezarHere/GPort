#pragma once
#include "gh.h"

enum { Path_BodyMaxLength = 4096, Path_HeadMaxLength = 256 };

typedef struct __impl_path_t
{
	CHAR tail;
	const CHAR *body;
	const CHAR *head;
} path_t;


extern CHAR *path_flatten(const path_t *path);
extern path_t path_create(const CHAR *filepath);

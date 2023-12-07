#pragma once
#include "quickfile.h"

typedef struct
{
	size_t values_ln;
	struct
	{
		nchar_t *kv[2];
	} *values;

} SettingFile_t;

// currentyl, setting files only accept 1-byte ascii
extern SettingFile_t stgf_fload(FILE *f);

// internally, 'src' will be converted to ascii
extern SettingFile_t stgf_fread(const nchar_t *src);

extern void stgf_close(SettingFile_t *stgf);


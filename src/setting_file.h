#pragma once
#include "quickfile.h"
typedef char stgf_char_t;

struct stgf_KeyValueSectorGroup;

typedef struct
{
	size_t values_ln;
	struct
	{
		nchar_t *key;
		nchar_t *value;
	} *values;
	struct stgf_KeyValueSectorGroup *_sectors;
} SettingFile_t;

// currentyl, setting files only accept 1-byte ascii
extern SettingFile_t stgf_fread(FILE *f);
extern SettingFile_t stgf_read(const nchar_t *src);


// internally, 'src' will be converted to ascii
extern void stgf_fwrite(SettingFile_t *stgf, FILE *fp);

extern stgf_char_t *stgf_get(const SettingFile_t *stgf);

extern void stgf_add(SettingFile_t *stgf, const nchar_t *key, const nchar_t *value);
extern void stgf_add_n(SettingFile_t *stgf, const stgf_char_t *key, const stgf_char_t *value);

extern void stgf_close(SettingFile_t *stgf);


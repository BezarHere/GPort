#pragma once
#include "quickfile.h"
typedef char stgf_char_t;

struct stgf_KeyValuePoolPair
{
	struct NCArray keys, values; 
};

typedef struct
{
	size_t values_ln;
	struct
	{
		nchar_t *key;
		nchar_t *value;
	} *values;
	struct stgf_KeyValuePoolPair __pools; // private
} SettingFile_t;

// currentyl, setting files only accept 1-byte ascii
extern SettingFile_t stgf_fread(FILE *f);
extern SettingFile_t stgf_read(const nchar_t *src);


// internally, 'src' will be converted to ascii
extern void stgf_fwrite(const SettingFile_t *stgf, FILE *fp);

/// @param len if not null, it will be set to the str's length
extern nchar_t *stgf_repr(const SettingFile_t *stgf, size_t *len);

extern void stgf_add(SettingFile_t *const stgf, const nchar_t *key, const nchar_t *value);

extern void stgf_add_s(SettingFile_t *const stgf, const nchar_t *key, const size_t key_n, const nchar_t *value, const size_t value_n);
extern void stgf_add_ns(SettingFile_t *const stgf, const stgf_char_t *key, const size_t key_n, const stgf_char_t *value, const size_t value_n);

extern void stgf_close(SettingFile_t *stgf);


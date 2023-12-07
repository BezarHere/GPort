#include "setting_file.h"
#include "malloc.h"
enum { STGF_ChunkLen = 16, STGF_ErrorStrLen = 255 }; // each chunk consists of 16 key-value pairs
typedef char stgf_char_t;

static nchar_t g_ErrorStr[STGF_ErrorStrLen + 1] = {0};
#define STGF_PARSE_ERR(msg) ((void)nc_sprintf_s(g_ErrorStr, STGF_ErrorStrLen, STR("STGF_PARSE_ERR AT %d:%d: " msg "\n"), cur_line, i - cur_line_begin))

typedef struct
{
	size_t _len, _cap;
	stgf_char_t *data;
} stgf_KeyValueSector_t;

static inline bool stgf_is_digit(const stgf_char_t c)
{
	return (stgf_char_t)'0' <= c && c <= (stgf_char_t)'9';
}

static inline bool stgf_is_whitespace(const stgf_char_t c)
{
	/// NOTE: MAY BUG
	return c <= (stgf_char_t)' ';
}

static inline bool stgf_is_letter(const stgf_char_t c)
{
	/// FIXME: if setting files supported unicode, a-z won't suffice
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

static inline void stgf_parse(const stgf_char_t *src, const size_t src_n, SettingFile_t *setting)
{
	size_t values_cap = 32;

	setting->values_ln = 0;
	setting->values = calloc(values_cap, sizeof(*setting->values));
	ASSERT(setting->values != NULL);


	stgf_KeyValueSector_t kv_sector = {0};
	
	const stgf_char_t *src_ky = NULL;
	size_t src_ky_ln = 0;

	bool expecting_value = false;
	size_t cur_line = 0, cur_line_begin = 0;

	for (size_t i = 0; i < src_n; i++)
	{
		if (src[i] == (stgf_char_t)'\n')
		{
			if (expecting_value)
			{
				STGF_PARSE_ERR("Expecting value");
				break;
			}

			if (src_ky)
			{
				STGF_PARSE_ERR("Standalone key");
				break;
			}

			cur_line++;
			cur_line_begin = i + 1;
		}

		if (stgf_is_whitespace(src[i]))
			continue;
		
		if (expecting_value)
		{
			size_t anchor = i;
			bool in_qouts = false;
			for (i++; i < src_n; i++)
			{
				// escaped char
				if (src[i] == (stgf_char_t)'\\')
				{
					i++;
					continue;
				}

				if (src[i] == (stgf_char_t)'"')
				{
					// found closing qout
					if (in_qouts)
						break;


					// last char is double qouts, error
					if (i == src_n - 1)
					{
						STGF_PARSE_ERR("No closing qoute (EOF)");
						break;
					}

					anchor = i + 1;
					in_qouts = true;
				}

				if (in_qouts)
				{
					if (src[i] == (stgf_char_t)'\n')
					{
						STGF_PARSE_ERR("Qouts left open");
						break;
					}
					continue;
				}

				if (stgf_is_whitespace(src[i]))
				{
					break;
				}
			}

			if (setting->values_ln == values_cap)
			{
				values_cap *= 2;
				setting->values = realloc(setting->values, values_cap);
				ASSERT(setting->values != NULL);
			}

		}
		
		if (src_ky && src[i] == (stgf_char_t)':')
		{
			// now, get a value
			expecting_value = true;
		}
		
	}

}

SettingFile_t stgf_fload(FILE *f)
{
	const long long flen = fend(f);
	stgf_char_t *str = malloc((flen + 1) * sizeof(stgf_char_t));
	str[flen] = 0;

	size_t read_bytes = fread_s(str, flen, sizeof(stgf_char_t), flen, f);
	
	// even if the entire file isn't read, it's still kinda valid
	
	// if (read_bytes != flen)
	// {
	// 	// Error?
	// }

	SettingFile_t st = {0};
	stgf_parse(str, flen, &st);

	free(str);
	return st;
}

SettingFile_t stgf_fread(const nchar_t *src)
{
	const size_t src_len = nc_strlen(src);
	const void *src_v = src;
	const bool owened_src = sizeof(stgf_char_t) != src; 

	// For if nchar_t is wide string and stgf_char_t is a char or vice versa
	if (owened_src)
	{
		stgf_char_t *stgf_str = malloc(sizeof(stgf_char_t) * (src_len + 1));
		for (size_t i = 0; i < src_len; i++)
		{
			stgf_str[i] = (stgf_char_t)src[i];
		}
		src_v = stgf_str;
	}

	SettingFile_t st = {0};
	stgf_parse(src_v, src_len, &st);
	
	if (owened_src)
		free(src);

	return st;
}

void stgf_close(SettingFile_t* stgf)
{
}

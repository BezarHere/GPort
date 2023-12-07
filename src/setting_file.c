#include "setting_file.h"
#include "malloc.h"

#define STGF_ERROR_STR_N 255
#define STGF_SECTOR_LEN 16
#define STGF_DEFAULT_SECTOR_CAP (STGF_SECTOR_LEN * 18)

typedef char stgf_char_t;

static nchar_t g_ErrorStr[STGF_ERROR_STR_N + 1] = {0};
#define STGF_PARSE_ERR(msg) ((void)nc_sprintf_s(g_ErrorStr, STGF_ERROR_STR_N, STR("STGF_PARSE_ERR AT %d:%d: " msg "\n"), cur_line, i - cur_line_begin))

typedef struct
{
	size_t _cap;
	size_t slots; // kv pairs currently set
	size_t size;
	stgf_char_t *data;
} stgf_KeyValueSector_t;

static inline rangei_t stgf_get_literal_strrange(const stgf_char_t *str, const size_t str_n, size_t cur_line, size_t cur_line_begin)
{
	rangei_t range = {0};
	bool in_qouts = false;
	size_t i;
	for (i = 0; i < str_n; i++)
	{
		// escaped char
		if (str[i] == (stgf_char_t)'\\')
		{
			i++;
			continue;
		}

		if (str[i] == (stgf_char_t)'"')
		{
			// found closing qout
			if (in_qouts)
				break;

			// last char is double qouts, error
			if (i == str_n - 1)
			{
				STGF_PARSE_ERR("No closing qoute (EOF)");
				break;
			}

			range.begin = i + 1;
			in_qouts = true;
			continue;
		}

		if (in_qouts)
		{
			if (str[i] == (stgf_char_t)'\n')
			{
				STGF_PARSE_ERR("Qouts left open");
				break;
			}
			continue;
		}

		if (stgf_is_whitespace(str[i]))
		{
			break;
		}
	}

	range.end = i;
	return range;
}

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

static inline stgf_KeyValueSector_t stgf_new_sector(size_t capacity)
{
	// might be bugged
	ASSERT(!(capacity & (size_t)-1));

	stgf_KeyValueSector_t sector;
	sector._cap = capacity;
	sector.slots = 0;
	sector.data = calloc(capacity, sizeof(stgf_char_t));
	sector.size = 0;
	
	ASSERT(sector.data != NULL);

	return sector;
}

static inline void stgf_expand_sector(stgf_KeyValueSector_t *sector, size_t new_capacity)
{
	sector->data = realloc(sector->data, new_capacity * sizeof(stgf_char_t));
	ASSERT(sector->data != NULL);
	
	memset(sector->data + sector->_cap, 0, new_capacity - sector->_cap);
	sector->_cap = new_capacity;
}


static inline void stgf_parse(const stgf_char_t *str, const size_t src_n, SettingFile_t *setting)
{
	size_t values_cap = 32;

	setting->values_ln = 0;
	setting->values = calloc(values_cap, sizeof(*setting->values));
	ASSERT(setting->values != NULL);

	stgf_KeyValueSector_t kv_sector = stgf_new_sector(STGF_DEFAULT_SECTOR_CAP);
	
	const stgf_char_t *src_ky = NULL;
	size_t src_ky_ln = 0;

	bool expecting_value = false;
	size_t cur_line = 0, cur_line_begin = 0;

	for (size_t i = 0; i < src_n; i++)
	{
		// new line hit
		if (str[i] == (stgf_char_t)'\n')
		{
			// no value after equal sigen in line
			if (expecting_value)
			{
				STGF_PARSE_ERR("Expecting value");
				break;
			}

			// no equal sigen in line, only a key
			if (src_ky)
			{
				STGF_PARSE_ERR("Standalone key");
				break;
			}

			cur_line++;
			cur_line_begin = i + 1;
			continue;
		}

		// whitespace skipped
		if (stgf_is_whitespace(str[i]))
			continue;
		
		// not a whitespace and expecting value, parse it
		if (expecting_value)
		{
			

			if (setting->values_ln == values_cap)
			{
				values_cap *= 2;
				setting->values = realloc(setting->values, values_cap);
				ASSERT(setting->values != NULL);
			}

			// full sector slots, create the new one
			if (kv_sector.slots == STGF_SECTOR_LEN)
			{
				kv_sector = stgf_new_sector(STGF_DEFAULT_SECTOR_CAP);
			}
			// full sector with more kv pairs to load, expand
			else if (kv_sector.size == kv_sector._cap)
			{
				stgf_expand_sector(&kv_sector, kv_sector._cap * 2);
			}

			continue;
		}
		
		if (src_ky)
		{
			if (str[i] == (stgf_char_t)'=')
			{
				// now, get a value
				expecting_value = true;
			}

			continue;
		}

		// read key

		
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
	(void)read_bytes;

	SettingFile_t st = {0};
	stgf_parse(str, flen, &st);

	free(str);
	return st;
}

SettingFile_t stgf_fread(const nchar_t *src)
{
	const size_t src_len = nc_strlen(src);
	const void *src_v = src;
	const bool owened_src = sizeof(stgf_char_t) != sizeof(*src); 
	void *owened_ptr = NULL;

	// For if nchar_t is wide string and stgf_char_t is a char or vice versa
	if (owened_src)
	{
		stgf_char_t *stgf_str = malloc(sizeof(stgf_char_t) * (src_len + 1));
		ASSERT(stgf_str != NULL);
		for (size_t i = 0; i < src_len; i++)
		{
			stgf_str[i] = (stgf_char_t)src[i];
		}
		src_v = stgf_str;
		owened_ptr = stgf_str;
	}

	SettingFile_t st = {0};
	stgf_parse(src_v, src_len, &st);
	
	if (owened_src)
		free(owened_ptr);

	return st;
}

void stgf_close(SettingFile_t* stgf)
{
}

#include "setting_file.h"
#include <malloc.h>
#include <math.h>
#include <stdbool.h>

#define STGF_ERROR_STR_N 255
#define STGF_SECTOR_LEN 16
#define STGF_DEFAULT_SECTOR_CAP (STGF_SECTOR_LEN * 18)

typedef struct stgf_KeyValueSector
{
	size_t _cap;
	size_t slots; // kv pairs currently set
	size_t size; // in stgf_char_t
	nchar_t *data;
} stgf_KeyValueSector_t;

typedef struct stgf_KeyValueSectorGroup
{
	size_t _cap, size;
	stgf_KeyValueSector_t *sectors;
} stgf_KeyValueSectorGroup_t;

static nchar_t g_ErrorStr[STGF_ERROR_STR_N + 1] = {0};

/// TODO: make this more strict
#define STGF_PARSE_ERR(msg, line, column) {((void)nc_sprintf_s(g_ErrorStr, STGF_ERROR_STR_N, STR("STGF_PARSE_ERR AT %lld:%lld: " msg "\n"), line, column)); nc_printf(g_ErrorStr); }


static inline void stgf_char_to_nchar_c(nchar_t *dst, const stgf_char_t *const src, const size_t src_len)
{
	if (sizeof(nchar_t) == sizeof(stgf_char_t))
	{
		memcpy(dst, src, src_len * sizeof(stgf_char_t));
		return;
	}

	for (size_t i = 0; i < src_len; i++)
		dst[i] = (nchar_t)src[i];
}

static inline void nchar_to_stgf_char_c(stgf_char_t *dst, const nchar_t *const src, const size_t src_len)
{
	if (sizeof(nchar_t) == sizeof(stgf_char_t))
	{
		memcpy(dst, src, src_len * sizeof(nchar_t));
		return;
	}

	for (size_t i = 0; i < src_len; i++)
		dst[i] = (stgf_char_t)src[i];
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

static inline rangei_t stgf_get_literal_strrange(const stgf_char_t *str, const size_t str_n, size_t cur_line, size_t cur_column)
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
				STGF_PARSE_ERR("No closing qoute (EOF)", cur_line, cur_column + i);
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
				STGF_PARSE_ERR("Qouts left open", cur_line, cur_column + i);
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


static inline stgf_KeyValueSector_t stgf_new_sector(size_t capacity)
{

	stgf_KeyValueSector_t sector;
	sector._cap = capacity;
	sector.slots = 0;
	sector.data = calloc(capacity, sizeof(nchar_t));
	sector.size = 0;
	
	ASSERT(sector.data != NULL);

	return sector;
}

static inline void stgf_expand_sector(stgf_KeyValueSector_t *sector, size_t new_capacity)
{
	sector->data = realloc(sector->data, new_capacity * sizeof(nchar_t));
	ASSERT(sector->data != NULL);
	
	memset(sector->data + sector->_cap, 0, new_capacity - sector->_cap);
	sector->_cap = new_capacity;
}

static inline size_t stgf_get_sectors_count(const SettingFile_t *setting)
{
	size_t i = setting->values_ln / STGF_SECTOR_LEN;
	return i + ((i * STGF_SECTOR_LEN == setting->values_ln) ? 0 : 1);
}

static inline stgf_KeyValueSector_t stgf_get_last_sector(const SettingFile_t *setting)
{
	stgf_KeyValueSector_t sector = {0};
	if (setting->values_ln == 0)
		return sector;
	
	size_t sectors_count = stgf_get_sectors_count(setting);
	sector.data = setting->values[(sectors_count - 1) * STGF_SECTOR_LEN].key;
	sector._cap
}

static inline void stgf_parse(const stgf_char_t *str, const size_t src_n, SettingFile_t *setting)
{
	size_t values_cap = 32;

	setting->values_ln = 0;
	setting->values = calloc(values_cap, sizeof(*setting->values));
	ASSERT(setting->values != NULL);

	stgf_KeyValueSector_t kv_sector = stgf_new_sector(STGF_DEFAULT_SECTOR_CAP);
	
	size_t key_index = 0;
	size_t key_ln = 0;

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
				STGF_PARSE_ERR("Expecting value", cur_line, i - cur_line_begin);
				break;
			}

			// no equal sigen in line, only a key
			if (key_ln)
			{
				STGF_PARSE_ERR("Standalone key", cur_line, i - cur_line_begin);
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
			// full sector slots, create the new one
			if (kv_sector.slots == STGF_SECTOR_LEN)
			{
				kv_sector = stgf_new_sector(STGF_DEFAULT_SECTOR_CAP);
			}

			const stgf_char_t *current_str_pos = str + i;

			// the value string start and end
			rangei_t value_range = stgf_get_literal_strrange(current_str_pos, src_n - i, cur_line, i - cur_line_begin);
			const size_t value_ln = value_range.end - value_range.begin;

			// value and key string sizes (+ their terminating nulls)
			const size_t kv_slot_size = value_ln + key_ln + 2;
			const size_t sector_space = kv_sector._cap - kv_sector.size;

			// keyvalue pair is too long to be saved, Expand
			if (kv_slot_size > sector_space)
			{
				size_t new_cap = kv_sector._cap * 2;
				while (new_cap - kv_sector.size < kv_slot_size)
				{
					new_cap *= 2;
					ASSERT(new_cap < (1 << 31));
				}

				stgf_expand_sector(&kv_sector, new_cap);
			}

			stgf_add_n(setting, key_index + str, value_range.begin + str);
			
			nchar_t *const key_slot = kv_sector.data + kv_sector.size;
			nchar_t *const value_slot = key_slot + key_ln + 1;

			stgf_char_to_nchar_c(key_slot, str + key_index, key_ln);
			key_slot[key_ln] = 0;

			stgf_char_to_nchar_c(value_slot, current_str_pos + value_range.begin, value_ln);
			value_slot[value_ln] = 0;

			// advance secotr
			kv_sector.size += kv_slot_size;
			kv_sector.slots++;

			setting->values[setting->values_ln].key = key_slot;
			setting->values[setting->values_ln].value = value_slot;
			setting->values_ln++;

			if (setting->values_ln == values_cap)
			{
				values_cap *= 2;
				setting->values = realloc(setting->values, values_cap);
				ASSERT(setting->values != NULL);
			}

			// clearing
			key_ln = 0;
			key_index = 0;
			expecting_value = false;

			i += value_range.end;
			continue;
		}
		
		if (key_ln)
		{
			if (str[i] != (stgf_char_t)'=')
				STGF_PARSE_ERR("Expected '=' after key", cur_line, i - cur_line_begin);
			
			// now, get a value
			expecting_value = true;

			continue;
		}

		// read key
		rangei_t key_range = stgf_get_literal_strrange(str + i, src_n - i, cur_line, i - cur_line_begin);
		key_index = i + key_range.begin;
		key_ln = key_range.end - key_range.begin;

		i += key_range.end;
	}

	// no values, free the kv sector created before the parsing
	if (setting->values_ln == 0)
		free(kv_sector.data);

}

SettingFile_t stgf_fread(FILE *f)
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

SettingFile_t stgf_read(const nchar_t *src)
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
	for (size_t i = 0; i < stgf->values_ln; i += STGF_SECTOR_LEN)
	{
		free(stgf->values[i].key); // free sector
		stgf->values[i].key = NULL;
	}
	free(stgf->values);
	stgf->values = NULL;
}

void stgf_fwrite(SettingFile_t *stgf, FILE *fp)
{

}

stgf_char_t *stgf_get(const SettingFile_t *stgf)
{

}

void stgf_add(SettingFile_t* stgf, const nchar_t* key, const nchar_t* value)
{
}

void stgf_add_n(SettingFile_t *stgf, const stgf_char_t *key, const stgf_char_t *value)
{
	
}

#include "setting_file.h"
#include <malloc.h>
#include <math.h>
#include <stdbool.h>


#define STGF_ERROR_STR_N 255
#define STGF_DEFAULT_SECTOR_CAP (STGF_SECTOR_LEN * 18)

typedef struct stgf_KeyValuePoolPair stgf_KeyValuePoolPair_t;


static nchar_t g_ErrorStr[STGF_ERROR_STR_N + 1] = {0};

/// TODO: make this more strict
#define STGF_PARSE_ERR(msg, line, column) {((void)nc_sprintf_s(g_ErrorStr, STGF_ERROR_STR_N, STR("STGF_PARSE_ERR AT %lld:%lld: " msg "\n"), line, column)); nc_printf(g_ErrorStr); }

/// @note Doesn't set a null termination 
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

/// @note Doesn't set a null termination
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

static inline void stgf_parse(const stgf_char_t *str, const size_t src_n, SettingFile_t *setting)
{
	size_t values_cap = 32;

	setting->values_ln = 0;
	setting->values = calloc(values_cap, sizeof(*setting->values));
	ASSERT(setting->values != NULL);

	struct NCArray *keys_pool = &setting->__pools.keys, *values_pool = &setting->__pools.values;
	nc_init_arr(keys_pool);
	nc_init_arr(values_pool);

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
			const stgf_char_t *current_str_pos = str + i;

			// the value string start and end
			rangei_t value_range = stgf_get_literal_strrange(current_str_pos, src_n - i, cur_line, i - cur_line_begin);
			const size_t value_ln = value_range.end - value_range.begin;

			// where the key and value string start
			nchar_t *key_pslot = keys_pool->data + keys_pool->size;
			nchar_t *value_pslot = values_pool->data + values_pool->size;
			
			// add the key and the value
			stgf_add_ns(setting, key_index + str, key_ln, value_range.begin + str, value_ln);

			// setting the entry
			setting->values[setting->values_ln].key = key_pslot;
			setting->values[setting->values_ln].value = value_pslot;
			setting->values_ln++;

			// expand if data is full
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
}

SettingFile_t stgf_fread(FILE *f)
{
	const long long flen = fend(f);
	stgf_char_t *str = calloc(flen + 1, sizeof(stgf_char_t));

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

	// if nchar_t and stgf_char_t aren't compatible
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
	free(stgf->__pools.keys.data);
	stgf->__pools.keys._cap = 0;
	stgf->__pools.keys.data = NULL;

	free(stgf->__pools.values.data);
	stgf->__pools.values._cap = 0;
	stgf->__pools.values.data = NULL;

	free(stgf->values);
	stgf->values = NULL;
}

void stgf_fwrite(const SettingFile_t *stgf, FILE *fp)
{
	size_t repr_len;
	nchar_t *repr = stgf_repr(stgf, &repr_len);
	stgf_char_t *sstr = calloc(repr_len, sizeof(stgf_char_t));
	ASSERT(sstr != NULL);

	nchar_to_stgf_char_c(sstr, repr, repr_len);
	
	free(repr);

	size_t bytes_written = fwrite(sstr, sizeof(nchar_t), repr_len, fp);

	free(sstr);

	ASSERT(bytes_written == repr_len);
}

nchar_t *stgf_repr(const SettingFile_t *stgf, size_t *len)
{
	struct NCArray arr;
	nc_init_arr(&arr);

	for (size_t i = 0; i < stgf->values_ln; i++)
	{
		const size_t kn = nc_strlen(stgf->values[i].key);
		const size_t vn = nc_strlen(stgf->values[i].value);
		const size_t lineln = kn + 4 + vn + 2;

		if (NC_ARR_SPACE(&arr) < lineln)
			nc_growfor_arr(&arr, lineln);
		
		(void)memcpy(arr.data + arr.size, stgf->values[i].key, sizeof(nchar_t) * kn);
		arr.size += kn;

		arr.data[0] = ' ';
		arr.data[1] = '=';
		arr.data[2] = ' ';
		arr.data[3] = '"';

		arr.size += 4;

		(void)memcpy(arr.data + arr.size, stgf->values[i].value, sizeof(nchar_t) * vn);
		arr.size += vn;

		arr.data[0] = '"';
		arr.data[1] = '\n';

		arr.size += 2;
	}

	if (len)
	 *len = arr.size;
	return arr.data;
}

void stgf_add(SettingFile_t *const stgf, const nchar_t* key, const nchar_t* value)
{
#ifdef PARANOID
	ASSERT(stgf != NULL);
	ASSERT(key != NULL);
	ASSERT(value != NULL);
#endif
	stgf_KeyValuePoolPair_t *const pool = &stgf->__pools;
	size_t i; 
	for (i = 0; key[i]; i++)
	{
		if (pool->keys.size == pool->keys._cap)
			nc_grow_arr(&pool->keys);
		pool->keys.data[pool->keys.size + i] = key[i];
	}
	pool->keys.data[i + pool->keys.size] = 0;
	pool->keys.size += i + 1; // +1 for null sepration
	
	for (i = 0; value[i]; i++)
	{
		if (pool->values.size == pool->values._cap)
			nc_grow_arr(&pool->values);
		pool->values.data[pool->values.size + i] = value[i];
	}
	pool->values.data[i + pool->values.size] = 0;
	pool->values.size += i + 1; // +1 for null sepration
}

void stgf_add_s(SettingFile_t *const stgf, const nchar_t* key, const size_t key_n, const nchar_t* value, const size_t value_n)
{
#ifdef PARANOID
	ASSERT(stgf != NULL);
	ASSERT(key != NULL);
	ASSERT(value != NULL);
#endif
	stgf_KeyValuePoolPair_t *const pool = &stgf->__pools; 

	// expand key pool if it has insufficent space
	if (NC_ARR_SPACE(&pool->keys) < key_n + 1)
		nc_growfor_arr(&pool->keys, key_n + 1);

	// expand values pool if it has insufficent space
	if (NC_ARR_SPACE(&pool->values) < value_n + 1)
		nc_growfor_arr(&pool->values, value_n + 1);
	
	(void)nc_strcpy_s(pool->keys.data + pool->keys.size, key_n, key);
	(void)nc_strcpy_s(pool->values.data + pool->values.size, value_n, value);

	pool->keys.data[key_n] = pool->values.data[value_n] = 0;

	pool->keys.size += key_n + 1;
	pool->values.size += value_n + 1;
}

void stgf_add_ns(SettingFile_t *const stgf, const stgf_char_t* key, const size_t key_n, const stgf_char_t* value, const size_t value_n)
{
	if (sizeof(stgf_char_t) == sizeof(nchar_t))
		return stgf_add_s(stgf, (const nchar_t *)key, key_n, (const nchar_t *)value, value_n);
	nchar_t *nchar_key = calloc(key_n + 1, sizeof(nchar_t));
	ASSERT(nchar_key != NULL);

	nchar_t *nchar_value = calloc(value_n + 1, sizeof(nchar_t));
	ASSERT(nchar_value != NULL);


	stgf_char_to_nchar_c(nchar_key, key, key_n);
	stgf_char_to_nchar_c(nchar_value, value, value_n);

	nchar_key[key_n] = 0;
	nchar_value[value_n] = 0;

	stgf_add_s(stgf, nchar_key, key_n, nchar_value, value_n);

	free(nchar_key);
	free(nchar_value);
}

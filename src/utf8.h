#pragma once
#include <string.h>

/// TODO: complete those impls

typedef char charu8_t;

static inline int UTF8_LEN_LOOKUP[]
{
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 7, 8,
};


/// @param str a utf-8 string
/// @param str_ln the string's length in bytes
static inline size_t utf8_strlen_s(const charu8_t *str, const size_t str_ln)
{
	size_t len = 0;
	for (size_t i = 0; i < str_ln; i++)
	{
		if (!(str[i] & 128))
		{
			len++;
			continue;
		}

		const int codepoint_len = UTF8_LEN_LOOKUP[str[i] & 127];

		// misplaced continuation byte
		if (codepoint_len == 1)
		{
			/// FIXME: ERROR?
			len++;
			continue;
		}

		// reading multibyte codepoint, advance to next byte

		i++;

		for (int j = 0; j < codepoint_len; j++)
		{

			// Invalid continuation byte [first bit isn't on]
			if (!(str[i + j] & 128))
			{
				/// FIXME: ERROR?
			}

			// anything bigger will make the char a multibyte codepoint instead of continuation byte
			if (str[i + j] > 0b10111111)
			{
				/// FIXME: ERROR?
			}
		}

		// goto the end of the codepoint
		i += codepoint_len - 2; // <- NOTE: multibyte codepoints can't be less the 2 bytes long so this maynot bug 
		
	}
	return len;
}

#include "path.h"
#define NPOS ((size_t)-1)

enum { Path_SegMaxLen = 254, Path_MaxSegCount = 2048 };

static inline bool path_is_char_path_sep(CHAR c)
{
	return c == (CHAR)'\\' || c == (CHAR)'/';
}

static inline size_t path_segments_len(const path_t *path)
{
	if (path->_segments_count == 0)
		return 0;

	ASSERT(path->_segments[0] != NULL);

	size_t total = 0;
	for (size_t i = 0; i < path->_segments_count - 1; i++)
	{
		ASSERT(path->_segments[i + 1] != NULL);
		// overflow: corrupted segment
		// segment i + 1 beginning - 1 (for null term) should be the end of segment i
		ASSERT((path->_segments[i + 1] - 1) - path->_segments[i] < Path_SegMaxLen);
		total += path->_segments[i + 1] - path->_segments[i];
	}

	total += char_strlen_s(path->_segments[path->_segments_count - 1], Path_SegMaxLen);

	return total;
}

static inline size_t path_length(const path_t *path)
{
	const size_t segments_length = path_segments_len(path);

	if (segments_length == NPOS)
		return NPOS;

	return segments_length;
}

CHAR *path_flatten(const path_t *path)
{
	const size_t path_len = path_length(path);

	CHAR *str = malloc(sizeof(CHAR) * (path_len + 1));
	ASSERT(str != NULL);

	CHAR *dst_str = str;
	for (size_t i = 0; i < path->_segments_count; i++)
	{
		const size_t seg_len = char_strlen(path->_segments[i]);
		char_strcpy(dst_str, path->_segments[i]);
		if (i < path->_segments_count - 1)
		{
			dst_str[seg_len] = (CHAR)'\\';
		}
		dst_str += seg_len + 1;
	}

	str[path_len] = 0;
	return str;
}

path_t path_create(const CHAR *filepath)
{
	ASSERT(filepath != NULL);
	// limit too big, (practically) useless
	const size_t filepath_len = char_strlen_s(filepath, Path_SegMaxLen * Path_MaxSegCount);

	path_t path = {0};

	// worst case there is a seprator between each char + 1 for null terminating
	const size_t path_segments_pool_size_bytes = sizeof(CHAR) * (filepath_len * 2 + 1);
	CHAR *path_seg_pool = malloc(path_segments_pool_size_bytes);
	ASSERT(path_seg_pool != NULL);
	memset(path_seg_pool, 0, path_segments_pool_size_bytes);
	
	// worst case is each char is a segment
	path._segments = malloc(filepath_len * sizeof(CHAR *));
	ASSERT(path._segments != NULL);
	memset(path._segments, 0, filepath_len * sizeof(CHAR *));


	// breaking up the filepath to segments
	size_t src_segment_index = 0, path_seg_pool_index = 0;
	for (size_t i = 0; i < filepath_len; i++)
	{
		// drive letter segment
		// c:\ <- somthing like this (never used linux, maybe add a check for /dev/env or sm)
		if (filepath[i] == (CHAR)':' && path._segments_count == 0 && path_is_char_path_sep(filepath[i + 1]))
		{
			// advance to the char after the ':'
			i++;
		}

		if (path_is_char_path_sep(filepath[i]))
		{
			
			// stop double seprators from messing up the segments
			if (i != src_segment_index)
			{

				// copy to segments pool
				memcpy(path_seg_pool + path_seg_pool_index, filepath + src_segment_index, (i - src_segment_index) * sizeof(CHAR));

				// put a null termination at the end of the current segment's substr 
				path_seg_pool[path_seg_pool_index + (i - src_segment_index)] = 0;

				// setup the last segment to point to it's substr in the segment pool
				path._segments[path._segments_count] = path_seg_pool + path_seg_pool_index;

				// next substr in seg pool (+ 1 to skip the null termination)
				path_seg_pool_index += (i - src_segment_index) + 1;

				// next segment
				path._segments_count++;
			}

			src_segment_index = i + 1;
		}
	}

	// add last segment if not already added
	if (src_segment_index != filepath_len)
	{
		// copy to segments pool
		memcpy(path_seg_pool + path_seg_pool_index, filepath + src_segment_index, (filepath_len - src_segment_index) * sizeof(CHAR));

		// put a null termination at the end of the current segment's substr 
		path_seg_pool[path_seg_pool_index + (filepath_len - src_segment_index)] = 0;

		// setup the last segment to point to it's substr in the segment pool
		path._segments[path._segments_count] = path_seg_pool + path_seg_pool_index;

		// next substr in seg pool (+ 1 to skip the null termination)
		path_seg_pool_index += (filepath_len - src_segment_index) + 1;

		// next segment
		path._segments_count++;
	}


	// if all things go haywire, this will ensure the freeing of the path segment pool
	path._segments[0] = path_seg_pool;

	return path;
}

CHAR *path_parent(const path_t *path)
{

}

CHAR *path_filename(const path_t *path)
{

}

CHAR *path_extension(const path_t *path)
{

}

void path_close(path_t *path)
{
	// freeing the segment pool
	free(path->_segments[0]);
	path->_segments[0] = NULL;

	// freeing the segments array
	free(path->_segments);
	path->_segments = NULL;
}

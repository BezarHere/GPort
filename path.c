#include "path.h"
#define NPOS ((size_t)-1)

enum { Path_SegMaxLen = 254, Path_MaxSegCount = 2048 };

static inline bool path_is_char_path_sep(nchar_t c)
{
	return c == (nchar_t)'\\' || c == (nchar_t)'/';
}

static inline size_t path_segments_len(const path_t *path, const size_t start_seg, const size_t end_seg)
{
	if (end_seg <= start_seg)
		return 0;
	ASSERT(end_seg <= path->_segments_count);
	// null first segment means the pool is dead
	ASSERT(path->_segments != NULL && path->_segments[0] != NULL);

	size_t total = 0;
	for (size_t i = start_seg; i < end_seg - 1; i++)
	{
		ASSERT(path->_segments[i + 1] != NULL);
		// overflow: corrupted segment
		// segment i + 1 beginning - 1 (for null term) should be the end of segment i
		ASSERT((path->_segments[i + 1] - 1) - path->_segments[i] < Path_SegMaxLen);
		total += path->_segments[i + 1] - path->_segments[i];
	}

	total += nc_strlen_s(path->_segments[end_seg- 1], Path_SegMaxLen);

	return total;
}

static inline size_t path_length(const path_t *path)
{
	return path_segments_len(path, 0, path->_segments_count);
}

static inline nchar_t *path_flatten_ranged(const path_t *path, const size_t start_seg, const size_t end_seg)
{
	const size_t path_len = path_segments_len(path, start_seg, end_seg);

	nchar_t *str = malloc(sizeof(nchar_t) * (path_len + 1));
	ASSERT(str != NULL);

	nchar_t *dst_str = str;
	for (size_t i = start_seg; i < end_seg; i++)
	{
		const size_t seg_len = nc_strlen(path->_segments[i]);
		nc_strcpy(dst_str, path->_segments[i]);
		if (i < end_seg - 1)
		{
			dst_str[seg_len] = (nchar_t)'\\';
		}
		dst_str += seg_len + 1;
	}

	str[path_len] = 0;
	return str;
}

nchar_t *path_flatten(const path_t *path)
{
	return path_flatten_ranged(path, 0, path->_segments_count);
}

path_t path_create(const nchar_t *filepath)
{
	ASSERT(filepath != NULL);
	// limit too big, (practically) useless
	const size_t filepath_len = nc_strlen_s(filepath, Path_SegMaxLen * Path_MaxSegCount);

	path_t path = {0};

	// worst case there is a seprator between each char + 1 for null terminating
	const size_t path_segments_pool_size_bytes = sizeof(nchar_t) * (filepath_len * 2 + 1);
	nchar_t *path_seg_pool = malloc(path_segments_pool_size_bytes);
	ASSERT(path_seg_pool != NULL);
	memset(path_seg_pool, 0, path_segments_pool_size_bytes);
	
	// worst case is each char is a segment
	path._segments = malloc(filepath_len * sizeof(nchar_t *));
	ASSERT(path._segments != NULL);
	memset(path._segments, 0, filepath_len * sizeof(nchar_t *));


	// breaking up the filepath to segments
	size_t src_segment_index = 0, path_seg_pool_index = 0;
	for (size_t i = 0; i < filepath_len; i++)
	{
		// drive letter segment
		// c:\ <- somthing like this (never used linux, maybe add a check for /dev/env or sm)
		if (filepath[i] == (nchar_t)':' && path._segments_count == 0 && path_is_char_path_sep(filepath[i + 1]))
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
				memcpy(path_seg_pool + path_seg_pool_index, filepath + src_segment_index, (i - src_segment_index) * sizeof(nchar_t));

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
		memcpy(path_seg_pool + path_seg_pool_index, filepath + src_segment_index, (filepath_len - src_segment_index) * sizeof(nchar_t));

		// put a null termination at the end of the current segment's substr 
		path_seg_pool[path_seg_pool_index + (filepath_len - src_segment_index)] = 0;

		// setup the last segment to point to it's substr in the segment pool
		path._segments[path._segments_count] = path_seg_pool + path_seg_pool_index;

		// next substr in seg pool (+ 1 to skip the null termination)
		// path_seg_pool_index += (filepath_len - src_segment_index) + 1;

		// next segment
		path._segments_count++;
	}


	// if all things go haywire, this will ensure the freeing of the path segment pool
	path._segments[0] = path_seg_pool;

	return path;
}

nchar_t *path_parent(const path_t *path)
{
	return path_flatten_ranged(path, 0, path->_segments_count - 1);
}

nchar_t *path_file(const path_t *path)
{
	return path_flatten_ranged(path, path->_segments_count - 1, path->_segments_count);
}

nchar_t *path_filename(const path_t *path)
{
	nchar_t *filebasename = path_file(path);
	nchar_t *last_point = nc_strrchr(filebasename, (nchar_t)'.');
	
	// no extension, all filename
	if (last_point == NULL)
		return filebasename;

	if (last_point == filebasename)
		return NULL;

	nchar_t *filename = malloc(((last_point - filebasename) + 1) * sizeof(nchar_t));
	ASSERT(filename != NULL);

	str_psplit_2(filebasename, last_point - filebasename, filename, NULL);

	free(filebasename);

	return filename;
}

nchar_t *path_extension(const path_t *path)
{
	nchar_t *filebasename = path_file(path);
	nchar_t *last_point = nc_strrchr(filebasename, (nchar_t)'.');
	
	// no dot, no extension, all filename OR the dot is the last charecter so no extension too
	if (last_point == NULL || filebasename[(last_point - filebasename) + 1] == (nchar_t)0)
	{
		return NULL;
	}

	const size_t flen = nc_strlen(filebasename);

	nchar_t *extension = malloc(flen - ((last_point - filebasename) + 1) + 1);
	ASSERT(extension != NULL);

	str_psplit_2(filebasename, (last_point - filebasename) + 1, NULL, extension);

	free(filebasename);

	return extension;
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

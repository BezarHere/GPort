#pragma once
#include "gh.h"
#include "quickfile.h"

// how the path's segment's ptrs work:
// 0x44 0x41 ... 0x30 0x00 0x64 0x52 ...
// ^ seg[0]                ^ seg[1]
typedef struct __impl_path_t
{
	size_t _segments_count;
	nchar_t **_segments; // <- each pointer points to the total segments pool
} path_t;


extern nchar_t *path_flatten(const path_t *path);
extern path_t path_create(const nchar_t *filepath);

// [this\\is\\a_path]\\file.ext
extern nchar_t *path_parent(const path_t *path);
// this\\is\\a_path\\[file.ext]
extern nchar_t *path_file(const path_t *path);
// this\\is\\a_path\\[file].ext
extern nchar_t *path_filename(const path_t *path);
// this\\is\\a_path\\file.[ext]
extern nchar_t *path_extension(const path_t *path);

/// @note the actual path object isn't freed, ONLY the buffers and pools are freed
extern void path_close(path_t* path);

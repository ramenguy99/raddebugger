#undef internal

// FreeType headers
#include <ft2build.h>
#include FT_FREETYPE_H

struct FP_FreeType_State
{
  Arena *arena;
  FT_Library ft;
};

#define internal static
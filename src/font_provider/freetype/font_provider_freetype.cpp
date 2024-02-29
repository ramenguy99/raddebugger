
global FP_FreeType_State *fp_freetype_state = 0;

fp_hook void
fp_init(void)
{
  //- dmylo: initialize main state
  {
    Arena *arena = arena_alloc();
    fp_freetype_state = push_array(arena, FP_FreeType_State, 1);
    fp_freetype_state->arena = arena;
  }

  //- dmylo: initialize FreeType
  FT_Error error = FT_Init_FreeType(&fp_freetype_state->ft);
}

internal FP_Handle
fp_freetype_handle_from_font(FT_Face font)
{
  FP_Handle result = {0};
  result.u64[0] = (U64)font;
  return result;
}

internal FT_Face
fp_freetype_font_from_handle(FP_Handle font)
{
  FT_Face result = (FT_Face)font.u64[0];
  return result;
}

fp_hook FP_Handle
fp_font_open(String8 path)
{
  ProfBeginFunction();

  FT_Face font = {0};
  FT_Error error = FT_New_Face(fp_freetype_state->ft, (const char*)path.str, 0, &font);

  //- dmylo: handlify & return
  FP_Handle handle = fp_freetype_handle_from_font(font);

  ProfEnd();
  return handle;
}

fp_hook FP_Handle
fp_font_open_from_static_data_string(String8 *data_ptr)
{
  ProfBeginFunction();

  FT_Face font = {0};
  FT_Error error = FT_New_Memory_Face(fp_freetype_state->ft, data_ptr->str, data_ptr->size, 0, &font);

  //- dmylo: handlify & return
  FP_Handle handle = fp_freetype_handle_from_font(font);

  ProfEnd();
  return handle;
}

fp_hook void
fp_font_close(FP_Handle handle)
{
  ProfBeginFunction();

  FT_Face font = fp_freetype_font_from_handle(handle);
  FT_Error error = FT_Done_Face(font);

  ProfEnd();
}

fp_hook FP_Metrics
fp_metrics_from_font(FP_Handle handle)
{
  ProfBeginFunction();
  FT_Face font = fp_freetype_font_from_handle(handle);


  FP_Metrics result = {0};
  {
    result.design_units_per_em = (F32)font->units_per_EM;
    result.ascent  = (F32)font->ascender;
    result.descent = (F32)-font->descender;
    result.line_gap = (F32)(font->height - font->ascender + font->descender);

    //- dmylo: Get capital H height in design units. If for some reason loading
    // the letter H fails, use the difference between ascender and descender
    // which seems to be a reasonable compromise in the fonts we tested.
    FT_Error error = FT_Load_Char(font, 'H', FT_LOAD_NO_SCALE);
    if(!error)
    {
      result.capital_height = (F32)font->glyph->metrics.height;
    }
    else
    {
      result.capital_height = (F32)(font->ascender + font->descender);
    }
  }

  ProfEnd();

  return result;
}

fp_hook NO_ASAN FP_RasterResult
fp_raster(Arena *arena, FP_Handle font, F32 size, FP_RasterMode mode, String8 string)
{
  ProfBeginFunction();
  Temp scratch = scratch_begin(&arena, 1);
  String32 string32 = str32_from_8(scratch.arena, string);

  FT_Face face = fp_freetype_font_from_handle(font);


  F32 real_size = size * 96.0f / 72.0f;
  FT_Error error = FT_Set_Pixel_Sizes(face, real_size, real_size);

  //- dmylo: compute atlas bounding box
  S32 advance = 0;
  Vec2S16 atlas_dim = {0};
  {
    atlas_dim.y = (S16)((96.f/72.f) * size * (face->ascender - face->descender) / face->units_per_EM);

    for (U64 i = 0; i < string32.size; i++) {
      error = FT_Load_Char(face, string32.str[i], FT_LOAD_RENDER);

      FT_Bitmap *bitmap = &(face->glyph->bitmap);

      atlas_dim.x = Max(atlas_dim.x, (advance >> 6) + face->glyph->bitmap_left + bitmap->width + 1);
      advance += face->glyph->advance.x;
    }

    atlas_dim.x += 7;
    atlas_dim.x -= atlas_dim.x % 8;
    atlas_dim.x += 4;
    atlas_dim.y += 4;
  }

  //- dmylo: fill & return
  FP_RasterResult result = {0};
  {
    // dmylo: fill basics
    result.atlas_dim = atlas_dim;
    result.atlas = push_array(arena, U8, atlas_dim.x * atlas_dim.y * 4);
    result.advance = (F32)(advance >> 6);
    result.height = atlas_dim.y;

    // dmylo: draw glyph run
    U8 *out_data  = (U8 *)result.atlas;
    U64 out_pitch = atlas_dim.x * 4;
    U8 *out_end  = (U8 *)result.atlas + out_pitch * atlas_dim.y;

    Vec2S32 p = {1, atlas_dim.y - 2 - (S32)((96.f/72.f) * size * -face->descender / face->units_per_EM)};
    //- dmylo: If we offset the text up by one pixel we generally get something
    //  closer to DWrite for text, icons look slightly off though.
    p.y -= 1;

    S32 advance = 0;
    U64 color_sum = 0;

    for (U64 i = 0; i < string32.size; i++) {
      error = FT_Load_Char(face, string32.str[i], FT_LOAD_RENDER);

      FT_Bitmap *bitmap = &face->glyph->bitmap;
      U8 *in_data = bitmap->buffer;
      U8 *in_end = bitmap->buffer + bitmap->pitch * bitmap->rows;
      U64 in_pitch = bitmap->pitch;

      S32 xpos = p.x + (advance >> 6) + face->glyph->bitmap_left;
      S32 ypos = p.y - face->glyph->bitmap_top;

      for(U64 y = 0; y < bitmap->rows; y += 1)
      {
        if(ypos + y < 0 || ypos + y >= atlas_dim.y) continue;

        U8 *in_line = in_data + in_pitch * y;
        U8* out_line = out_data + out_pitch * (ypos + y);

        for(U64 x = 0; x < bitmap->width; x += 1)
        {
          if(xpos + x < 0 || xpos + x >= atlas_dim.x) continue;

          U8 *in_pixel = in_line + x;
          U8 *out_pixel = out_line + (xpos + x) * 4;

          //- dmylo: Ensure writes and reads are in bounds
          Assert(out_pixel >= out_data && out_pixel + 4 <= out_end);
          Assert(in_pixel >= in_data && in_pixel + 1 <= in_end);

          U8 in_pixel_byte = in_pixel[0];
          out_pixel[0] = 255;
          out_pixel[1] = 255;
          out_pixel[2] = 255;
          out_pixel[3] = in_pixel_byte;

          color_sum += in_pixel_byte;
        }
      }
      advance += face->glyph->advance.x;
    }
    if(color_sum == 0)
    {
      result.atlas_dim = v2s16(0, 0);
    }
  }

  scratch_end(scratch);
  ProfEnd();
  return result;
}

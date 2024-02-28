
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
  FT_Error err = FT_Init_FreeType(&fp_freetype_state->ft);
}

fp_hook FP_Handle
fp_font_open(String8 path)
{
    return {};
}

fp_hook FP_Handle
fp_font_open_from_static_data_string(String8 *data_ptr)
{
    return {};
}

fp_hook void
fp_font_close(FP_Handle handle)
{

}

fp_hook FP_Metrics
fp_metrics_from_font(FP_Handle font)
{
    return {};
}

fp_hook NO_ASAN FP_RasterResult
fp_raster(Arena *arena, FP_Handle font, F32 size, FP_RasterMode mode, String8 string)
{
    return {};
}

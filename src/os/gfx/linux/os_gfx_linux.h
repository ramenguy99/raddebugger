// Copyright (c) 2024 Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

#ifndef OS_GFX_LINUX_H
#define OS_GFX_LINUX_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <X11/Xresource.h>

#include "render/opengl/generated/render_opengl.meta.h"

//- dmylo: disable GL/gl.h include, since we have all we need already generated.
#define __gl_h_
#include <GL/gl.h>
#include <GL/glx.h>

typedef struct X11_Window X11_Window;
struct X11_Window
{
  X11_Window *next;
  X11_Window *prev;
  Window window;
  OS_WindowRepaintFunctionType *repaint;
  void *repaint_user_data;
  XIC input_context;
  /*
  WINDOWPLACEMENT last_window_placement;
  F32 dpi;
  B32 first_paint_done;
  B32 maximized;
  */
};

#endif // OS_GFX_LINUX_H

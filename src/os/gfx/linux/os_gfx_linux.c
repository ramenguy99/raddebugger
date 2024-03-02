// Copyright (c) 2024 Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

////////////////////////////////
//~ rjf: @os_hooks Main Initialization API (Implemented Per-OS)

// #include <X11/keysym.h>
// #include <X11/Xatom.h>
// #include <X11/Xresource.h>
// #include <X11/Xcursor/Xcursor.h>

global X11_Window *   x11_first_free_window = 0;
global X11_Window *   x11_first_window = 0;
global X11_Window *   x11_last_window = 0;
global OS_EventList   x11_event_list = {0};
global Arena *        x11_event_arena = 0;

global Display* x11_display;
global int x11_screen;
global Window x11_root;
global Atom x11_atom_wm_delete_window;
global XContext x11_context;

typedef GLXContext (glx_create_context_attribs_arb)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

internal void
os_graphical_init(void)
{
  XInitThreads();
  XrmInitialize();
  x11_display = XOpenDisplay(NULL);
  x11_screen = DefaultScreen(x11_display);
  x11_root = RootWindow(x11_display, x11_screen);
  // TODO: monitor and refresh rate stuff
  x11_context = XUniqueContext();
}

////////////////////////////////
//~ rjf: @os_hooks Clipboards (Implemented Per-OS)

internal void
os_set_clipboard_text(String8 string)
{
}

internal String8
os_get_clipboard_text(Arena *arena)
{
  return str8_zero();
}

////////////////////////////////
//~ rjf: @os_hooks Windows (Implemented Per-OS)
internal OS_Handle
os_window_from_x11_window(X11_Window* window)
{
  OS_Handle result = {0};
  result.u64[0] = (U64)window;
  return result;
}

internal X11_Window *
x11_window_from_os_window(OS_Handle handle)
{
  return (X11_Window *)PtrFromInt(handle.u64[0]);
}

internal X11_Window *
x11_allocate_window(void)
{
  X11_Window *result = x11_first_free_window;
  if(result == 0)
  {
    result = push_array(lnx_perm_arena, X11_Window, 1);
  }
  else
  {
    x11_first_free_window = x11_first_free_window->next;
    MemoryZeroStruct(result);
  }
  if(result)
  {
    DLLPushBack(x11_first_window, x11_last_window, result);
  }
  return result;
}

internal void
x11_free_window(X11_Window *window)
{
  XDestroyWindow(x11_display, window->window);
  DLLRemove(x11_first_window, x11_last_window, window);
  window->next = x11_first_free_window;
  x11_first_free_window = window;
}

internal OS_Handle
os_window_open(Vec2F32 resolution, String8 title_not_zero_terminated)
{
  OS_Handle handle = {1};

  GLint glx_attribs[] = {
      GLX_X_RENDERABLE    , True,
      GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
      GLX_RENDER_TYPE     , GLX_RGBA_BIT,
      GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
      GLX_RED_SIZE        , 8,
      GLX_GREEN_SIZE      , 8,
      GLX_BLUE_SIZE       , 8,
      GLX_ALPHA_SIZE      , 8,
      GLX_DEPTH_SIZE      , 24,
      GLX_STENCIL_SIZE    , 8,
      GLX_DOUBLEBUFFER    , True,
      GLX_SAMPLE_BUFFERS  , 1,
      GLX_SAMPLES         , 1,
      None
    };
    
	int fbcount;
	GLXFBConfig* fbc = glXChooseFBConfig(x11_display, x11_screen, glx_attribs, &fbcount);
  if (fbc == 0 || fbcount == 0) 
  {
		//TODO error
    NotImplemented;
	}
  
  GLXFBConfig best_fbc = fbc[0];
	XFree(fbc); // Make sure to free this!

  XVisualInfo* visual = glXGetVisualFromFBConfig(x11_display, best_fbc);
	if (visual == 0) {
		//TODO error
    NotImplemented;
	}
	
	if (visual->screen != x11_screen) {
		//TODO error
    NotImplemented;
	}


  XSetWindowAttributes window_attribs;
	window_attribs.border_pixel = BlackPixel(x11_display, x11_screen);
	window_attribs.background_pixel = WhitePixel(x11_display, x11_screen);
	window_attribs.override_redirect = True;
	window_attribs.colormap = XCreateColormap(x11_display, x11_root, visual->visual, AllocNone);
	window_attribs.event_mask = ExposureMask;
	Window window = XCreateWindow(x11_display, x11_root, 0, 0, resolution.x, resolution.y, 0, visual->depth, InputOutput, visual->visual, CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &window_attribs);

  // Set window title
  Temp scratch = scratch_begin(0, 0);
  String8 title = push_str8_copy(scratch.arena, title_not_zero_terminated);
  XStoreName(x11_display, window, (char*)title.str);
  scratch_end(scratch);

  // Redirect Close
	x11_atom_wm_delete_window = XInternAtom(x11_display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(x11_display, window, &x11_atom_wm_delete_window, 1);

  glx_create_context_attribs_arb* glXCreateContextAttribsARB = (glx_create_context_attribs_arb*)glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );
	
	int context_attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
		GLX_CONTEXT_MINOR_VERSION_ARB, 1,
		GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		None
	};

	GLXContext context = 0;
  context = glXCreateContextAttribsARB(x11_display, best_fbc, 0, true, context_attribs);
	XSync(x11_display, False);

	Bool result = glXMakeCurrent(x11_display, window, context);

  if(!result)
  {
    // TODO:error
    NotImplemented;
  }

  //- dmylo: make/fill window
  X11_Window *x11_window = x11_allocate_window();
  {
    x11_window->window = window;
  }

  XSaveContext(x11_display, window, x11_context, (XPointer)x11_window);

  return os_window_from_x11_window(x11_window);
}

internal void
os_window_close(OS_Handle handle)
{
  X11_Window* window = x11_window_from_os_window(handle);
  x11_free_window(window);
}

internal void
os_window_first_paint(OS_Handle handle)
{
  X11_Window* window = x11_window_from_os_window(handle);
	XClearWindow(x11_display, window->window);
	XMapRaised(x11_display, window->window);
  // TODO: Maximize if requested
}

internal void
os_window_equip_repaint(OS_Handle handle, OS_WindowRepaintFunctionType *repaint, void *user_data)
{
  X11_Window *window = x11_window_from_os_window(handle);
  window->repaint = repaint;
  window->repaint_user_data = user_data;
}

internal void
os_window_focus(OS_Handle window)
{
  //TODO
}

internal B32
os_window_is_focused(OS_Handle window)
{
  //TODO
  return 0;
}

internal B32
os_window_is_fullscreen(OS_Handle window)
{
  //TODO
  return 0;
}

internal void
os_window_set_fullscreen(OS_Handle window, B32 fullscreen)
{
  //TODO
}

internal B32
os_window_is_maximized(OS_Handle window)
{
  //TODO
  return 0;
}

internal void
os_window_set_maximized(OS_Handle window, B32 maximized)
{
  //TODO
}

internal void
os_window_bring_to_front(OS_Handle window)
{
  //TODO
}

internal void
os_window_set_monitor(OS_Handle window, OS_Handle monitor)
{
  //TODO
}

internal Rng2F32
os_rect_from_window(OS_Handle window)
{
  Rng2F32 rect = r2f32(v2f32(0, 0), v2f32(500, 500));
  return rect;
}

internal Rng2F32
os_client_rect_from_window(OS_Handle window)
{
  Rng2F32 rect = r2f32(v2f32(0, 0), v2f32(500, 500));
  return rect;
}

internal F32
os_dpi_from_window(OS_Handle window)
{
  return 96.f;
}

////////////////////////////////
//~ rjf: @os_hooks Monitors (Implemented Per-OS)

internal OS_HandleArray
os_push_monitors_array(Arena *arena)
{
  OS_HandleArray arr = {0};
  return arr;
}

internal OS_Handle
os_primary_monitor(void)
{
  OS_Handle handle = {1};
  return handle;
}

internal OS_Handle
os_monitor_from_window(OS_Handle window)
{
  OS_Handle handle = {1};
  return handle;
}

internal String8
os_name_from_monitor(Arena *arena, OS_Handle monitor)
{
  return str8_zero();
}

internal Vec2F32
os_dim_from_monitor(OS_Handle monitor)
{
  Vec2F32 v = v2f32(1000, 1000);
  return v;
}

////////////////////////////////
//~ rjf: @os_hooks Events (Implemented Per-OS)

internal void
os_send_wakeup_event(void)
{
  //TODO
}

internal OS_Event *
x11_push_event(OS_EventKind kind, X11_Window *window)
{
  OS_Event *result = push_array(x11_event_arena, OS_Event, 1);
  DLLPushBack(x11_event_list.first, x11_event_list.last, result);
  result->timestamp_us = os_now_microseconds();
  result->kind = kind;
  result->window = os_window_from_x11_window(window);
  result->flags = os_get_event_flags();
  x11_event_list.count += 1;
  return(result);
}

// #error TODO:
// - get window from event, likely doable
// - handle window close
// - handle other types of events (see win32 impl)
// - make opengl stuff cross platform and render!
// - everything else (resize, keyboard and mouse events, dpi, monitor, clipboard, etc..)

internal OS_EventList
os_get_events(Arena *arena, B32 wait)
{
  x11_event_arena = arena;
  MemoryZeroStruct(&x11_event_list);

  bool has_events = XPending(x11_display) > 0;
  if(wait || has_events)
  {
    while(XPending(x11_display) > 0) 
    {
      XEvent ev;
      XNextEvent(x11_display, &ev);

      Window x11_window = ev.xany.window;

      XPointer user_data;
      XFindContext(x11_display, x11_window, x11_context, &user_data);
      X11_Window *window = (X11_Window *)user_data;
      if(!window) continue;

      // Resize event
      switch (ev.type)
      {
      case Expose: {
        // XWindowAttributes attribs;
        // XGetWindowAttributes(x11_display, ev.window, &attribs);
        // Resize(attribs.width, attribs.height);
        if(window->repaint != 0)
        {
          window->repaint(os_window_from_x11_window(window), window->repaint_user_data);
        }
      } break;

      case ClientMessage: {
				if (ev.xclient.data.l[0] == x11_atom_wm_delete_window) {
          x11_push_event(OS_EventKind_WindowClose, window);
				}
			} break;

			case DestroyNotify: { 
        x11_push_event(OS_EventKind_WindowClose, 0);
			} break;
      }
    }
  }

  return x11_event_list;
}

internal OS_EventFlags
os_get_event_flags(void)
{
  OS_EventFlags f = 0;
  return f;
}

internal B32
os_key_is_down(OS_Key key)
{
  return 0;
}

internal Vec2F32
os_mouse_from_window(OS_Handle window)
{
  return v2f32(0, 0);
}

////////////////////////////////
//~ rjf: @os_hooks Cursors (Implemented Per-OS)

internal void
os_set_cursor(OS_Cursor cursor)
{
}

////////////////////////////////
//~ rjf: @os_hooks System Properties (Implemented Per-OS)

internal F32
os_double_click_time(void)
{
  return 1.f;
}

internal F32
os_caret_blink_time(void)
{
  return 1.f;
}

internal F32
os_default_refresh_rate(void)
{
  return 60.f;
}

internal B32
os_granular_sleep_enabled(void)
{
  return 1;
}

////////////////////////////////
//~ rjf: @os_hooks Native Messages & Panics (Implemented Per-OS)

internal void
os_graphical_message(B32 error, String8 title, String8 message)
{
  printf("[%s] %.*ss: %.*s", error ? "Error" : "Info", (U32)title.size, (char*)title.str, (U32)message.size, (char*)message.str);
}

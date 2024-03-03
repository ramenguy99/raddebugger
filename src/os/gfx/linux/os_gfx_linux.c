// Copyright (c) 2024 Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

////////////////////////////////
//~ rjf: @os_hooks Main Initialization API (Implemented Per-OS)

#include <X11/keysym.h>
#include <X11/XKBlib.h>
// #include <X11/extensions/XKBstr.h>
// #include <X11/Xatom.h>
// #include <X11/Xresource.h>
// #include <X11/Xcursor/Xcursor.h>

global X11_Window *   x11_first_free_window = 0;
global X11_Window *   x11_first_window = 0;
global X11_Window *   x11_last_window = 0;
global OS_EventList   x11_event_list = {0};
global Arena *        x11_event_arena = 0;

global Display* x11_display;
global Screen* x11_screen_ptr;
global int x11_screen;
global Window x11_root;
global Atom x11_atom_wm_delete_window;
global XContext x11_context;

// Keyboard
global OS_Key x11_scancode_to_os_key_table[256];
global KeySym x11_os_key_to_scancode_table[OS_Key_COUNT];

// Input method
global XIM x11_input_method;

// OpenGL specific
global GLXContext x11_gl_context;
global GLXFBConfig x11_fbconfig;
global Window x11_dummy_window;

typedef GLXContext (glx_create_context_attribs_arb)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

// KeySym to OS_Key adapted from GLFW
internal OS_Key
x11_translate_key_sym(KeySym* keysyms, int count)
{
  if (count > 1)
  {
      switch (keysyms[1])
      {
          case XK_KP_0:           return OS_Key_Num0;
          case XK_KP_1:           return OS_Key_Num1;
          case XK_KP_2:           return OS_Key_Num2;
          case XK_KP_3:           return OS_Key_Num3;
          case XK_KP_4:           return OS_Key_Num4;
          case XK_KP_5:           return OS_Key_Num5;
          case XK_KP_6:           return OS_Key_Num6;
          case XK_KP_7:           return OS_Key_Num7;
          case XK_KP_8:           return OS_Key_Num8;
          case XK_KP_9:           return OS_Key_Num9;
          case XK_KP_Separator:
          case XK_KP_Decimal:     return OS_Key_NumPeriod;
          // case XK_KP_Equal:       return OS_Key_NumEqual;
          // case XK_KP_Enter:       return OS_Key_NumReturn;
          default:                break;
      }
  }

  switch (keysyms[0])
  {
      case XK_Escape:         return OS_Key_Esc;
      case XK_Tab:            return OS_Key_Tab;
      case XK_Shift_L:        return OS_Key_Shift;
      case XK_Shift_R:        return OS_Key_Shift;
      case XK_Control_L:      return OS_Key_Ctrl;
      case XK_Control_R:      return OS_Key_Ctrl;
      case XK_Meta_L:
      case XK_Alt_L:          return OS_Key_Alt;
      case XK_Mode_switch:      // Mapped to Alt_R on many keyboards
      case XK_ISO_Level3_Shift: // AltGr on at least some machines
      case XK_Meta_R:
      case XK_Alt_R:          return OS_Key_Alt;
      // case XK_Super_L:        return OS_Key_Super;
      // case XK_Super_R:        return OS_Key_Super;
      case XK_Menu:           return OS_Key_Alt;
      case XK_Num_Lock:       return OS_Key_NumLock;
      case XK_Caps_Lock:      return OS_Key_CapsLock;
      // case XK_Print:          return OS_Key_PrintScreen;
      case XK_Scroll_Lock:    return OS_Key_ScrollLock;
      case XK_Pause:          return OS_Key_Pause;
      case XK_Delete:         return OS_Key_Delete;
      case XK_BackSpace:      return OS_Key_Backspace;
      case XK_Return:         return OS_Key_Return;
      case XK_Home:           return OS_Key_Home;
      case XK_End:            return OS_Key_End;
      case XK_Page_Up:        return OS_Key_PageUp;
      case XK_Page_Down:      return OS_Key_PageDown;
      case XK_Insert:         return OS_Key_Insert;
      case XK_Left:           return OS_Key_Left;
      case XK_Right:          return OS_Key_Right;
      case XK_Down:           return OS_Key_Down;
      case XK_Up:             return OS_Key_Up;
      case XK_F1:             return OS_Key_F1;
      case XK_F2:             return OS_Key_F2;
      case XK_F3:             return OS_Key_F3;
      case XK_F4:             return OS_Key_F4;
      case XK_F5:             return OS_Key_F5;
      case XK_F6:             return OS_Key_F6;
      case XK_F7:             return OS_Key_F7;
      case XK_F8:             return OS_Key_F8;
      case XK_F9:             return OS_Key_F9;
      case XK_F10:            return OS_Key_F10;
      case XK_F11:            return OS_Key_F11;
      case XK_F12:            return OS_Key_F12;
      case XK_F13:            return OS_Key_F13;
      case XK_F14:            return OS_Key_F14;
      case XK_F15:            return OS_Key_F15;
      case XK_F16:            return OS_Key_F16;
      case XK_F17:            return OS_Key_F17;
      case XK_F18:            return OS_Key_F18;
      case XK_F19:            return OS_Key_F19;
      case XK_F20:            return OS_Key_F20;
      case XK_F21:            return OS_Key_F21;
      case XK_F22:            return OS_Key_F22;
      case XK_F23:            return OS_Key_F23;
      case XK_F24:            return OS_Key_F24;
      // case XK_F25:            return OS_Key_F25;

      // Numeric keypad
      case XK_KP_Divide:      return OS_Key_NumSlash;
      case XK_KP_Multiply:    return OS_Key_NumStar;
      case XK_KP_Subtract:    return OS_Key_NumMinus;
      case XK_KP_Add:         return OS_Key_NumPlus;

      // These should have been detected in secondary keysym test above!
      case XK_KP_Insert:      return OS_Key_Num0;
      case XK_KP_End:         return OS_Key_Num1;
      case XK_KP_Down:        return OS_Key_Num2;
      case XK_KP_Page_Down:   return OS_Key_Num3;
      case XK_KP_Left:        return OS_Key_Num4;
      case XK_KP_Right:       return OS_Key_Num6;
      case XK_KP_Home:        return OS_Key_Num7;
      case XK_KP_Up:          return OS_Key_Num8;
      case XK_KP_Page_Up:     return OS_Key_Num9;
      case XK_KP_Delete:      return OS_Key_NumPeriod;
      // case XK_KP_Equal:       return OS_Key_NumEqual;
      // case XK_KP_Enter:       return OS_Key_NumReturn;

      // Last resort: Check for printable keys (should not happen if the XKB
      // extension is available). This will give a layout dependent mapping
      // (which is wrong, and we may miss some keys, especially on non-US
      // keyboards), but it's better than nothing...
      case XK_a:              return OS_Key_A;
      case XK_b:              return OS_Key_B;
      case XK_c:              return OS_Key_C;
      case XK_d:              return OS_Key_D;
      case XK_e:              return OS_Key_E;
      case XK_f:              return OS_Key_F;
      case XK_g:              return OS_Key_G;
      case XK_h:              return OS_Key_H;
      case XK_i:              return OS_Key_I;
      case XK_j:              return OS_Key_J;
      case XK_k:              return OS_Key_K;
      case XK_l:              return OS_Key_L;
      case XK_m:              return OS_Key_M;
      case XK_n:              return OS_Key_N;
      case XK_o:              return OS_Key_O;
      case XK_p:              return OS_Key_P;
      case XK_q:              return OS_Key_Q;
      case XK_r:              return OS_Key_R;
      case XK_s:              return OS_Key_S;
      case XK_t:              return OS_Key_T;
      case XK_u:              return OS_Key_U;
      case XK_v:              return OS_Key_V;
      case XK_w:              return OS_Key_W;
      case XK_x:              return OS_Key_X;
      case XK_y:              return OS_Key_Y;
      case XK_z:              return OS_Key_Z;
      case XK_1:              return OS_Key_1;
      case XK_2:              return OS_Key_2;
      case XK_3:              return OS_Key_3;
      case XK_4:              return OS_Key_4;
      case XK_5:              return OS_Key_5;
      case XK_6:              return OS_Key_6;
      case XK_7:              return OS_Key_7;
      case XK_8:              return OS_Key_8;
      case XK_9:              return OS_Key_9;
      case XK_0:              return OS_Key_0;
      case XK_space:          return OS_Key_Space;
      case XK_minus:          return OS_Key_Minus;
      case XK_equal:          return OS_Key_Equal;
      case XK_bracketleft:    return OS_Key_LeftBracket;
      case XK_bracketright:   return OS_Key_RightBracket;
      case XK_backslash:      return OS_Key_BackSlash;
      case XK_semicolon:      return OS_Key_Semicolon;
      case XK_apostrophe:     return OS_Key_Quote;
      case XK_grave:          return OS_Key_Tick;
      case XK_comma:          return OS_Key_Comma;
      case XK_period:         return OS_Key_Period;
      case XK_slash:          return OS_Key_Slash;
      // case XK_less:           return OS_Key_; // At least in some layouts...
      default:                break;
  }

    // No matching translation was found
    return OS_Key_Null;
}

internal void
x11_input_context_destroy_callback(XIC ic, XPointer client_data, XPointer call_data)
{
  X11_Window *window = (X11_Window *)client_data;
  window->input_context = NULL;
}

internal void
x11_create_input_context(X11_Window *window)
{
  XIMCallback callback;
  callback.callback = (XIMProc)x11_input_context_destroy_callback;
  callback.client_data = (XPointer)window;

  window->input_context = XCreateIC(x11_input_method, XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
                                    XNClientWindow, window->window, XNFocusWindow, window->window, XNDestroyCallback, &callback, NULL);

  if (window->input_context)
  {
    XWindowAttributes attribs;
    XGetWindowAttributes(x11_display, window->window, &attribs);

    unsigned long filter = 0;
    if (XGetICValues(window->input_context, XNFilterEvents, &filter, NULL) == NULL)
    {
      XSelectInput(x11_display, window->window, attribs.your_event_mask | filter);
    }
  }
}

internal void
x11_input_method_destroy_callback(XIM im, XPointer client_data, XPointer callback_data)
{
  x11_input_method = NULL;
}

internal void 
x11_input_method_instantiate_callback(Display* display, XPointer client_data, XPointer call_data)
{
    if (x11_input_method) return;

    x11_input_method = XOpenIM(x11_display, 0, NULL, NULL);
    if (x11_input_method)
    {
        XIMCallback callback;
        callback.callback = (XIMProc)x11_input_method_destroy_callback;
        callback.client_data = NULL;
        XSetIMValues(x11_input_method, XNDestroyCallback, &callback, NULL);

        for(X11_Window *w = x11_first_window; w; w = w->next)
        {
            x11_create_input_context(w);
        }
    }
}

internal void
os_graphical_init(void)
{
  XInitThreads();
  XrmInitialize();
  x11_display = XOpenDisplay(NULL);
  x11_screen_ptr = DefaultScreenOfDisplay(x11_display);
  x11_screen = DefaultScreen(x11_display);
  x11_root = RootWindow(x11_display, x11_screen);
  x11_context = XUniqueContext();

  //- dmylo: Initialize key tables
  {
    int scancode_min = 0;
    int scancode_max = 0;
    XDisplayKeycodes(x11_display, &scancode_min, &scancode_max);

    int keysyms_per_keycode;
    KeySym* keysyms = XGetKeyboardMapping(x11_display, scancode_min, scancode_max - scancode_min + 1, &keysyms_per_keycode);

    for (int scancode = scancode_min;  scancode <= scancode_max;  scancode++)
    {
        U64 base = (U64)(scancode - scancode_min) * (U64)keysyms_per_keycode;
        OS_Key key = x11_translate_key_sym(&keysyms[base], keysyms_per_keycode);
        x11_scancode_to_os_key_table[scancode] = key;
        x11_os_key_to_scancode_table[key] = scancode;
    }

    XFree(keysyms);
  }

  //- dmylo: Initialize InputMethod for utf8 input
  if (XSupportsLocale())
  {
    XSetLocaleModifiers("");
    XRegisterIMInstantiateCallback(x11_display, NULL, NULL, NULL, x11_input_method_instantiate_callback, NULL);
  }
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
  XVisualInfo* visual = glXGetVisualFromFBConfig(x11_display, x11_fbconfig);
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
	window_attribs.event_mask = 
    ExposureMask | 
    ButtonPressMask |
    ButtonReleaseMask |
    KeyPressMask |
    KeyReleaseMask |
    PointerMotionMask |
    0;
	Window window = XCreateWindow(x11_display, x11_root, 0, 0,
  resolution.x, resolution.y,
  0, visual->depth, InputOutput, visual->visual, CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &window_attribs);

  // Set window title
  Temp scratch = scratch_begin(0, 0);
  String8 title = push_str8_copy(scratch.arena, title_not_zero_terminated);
  XStoreName(x11_display, window, (char*)title.str);
  scratch_end(scratch);

  // Redirect Close
	x11_atom_wm_delete_window = XInternAtom(x11_display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(x11_display, window, &x11_atom_wm_delete_window, 1);

  //- dmylo: make/fill window
  X11_Window *x11_window = x11_allocate_window();
  {
    x11_window->window = window;
  }

  XSaveContext(x11_display, window, x11_context, (XPointer)x11_window);

  //- dmylo: input context for char input
  x11_create_input_context(x11_window);

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
  // TODO: Maximize if requested -> set window size as monitor size
}

internal void
os_window_equip_repaint(OS_Handle handle, OS_WindowRepaintFunctionType *repaint, void *user_data)
{
  X11_Window *window = x11_window_from_os_window(handle);
  window->repaint = repaint;
  window->repaint_user_data = user_data;
}

internal void
os_window_focus(OS_Handle handle)
{
  X11_Window *window = x11_window_from_os_window(handle);
  XSetInputFocus(x11_display, window->window, RevertToNone, CurrentTime);
}

internal B32
os_window_is_focused(OS_Handle handle)
{
  X11_Window *window = x11_window_from_os_window(handle);

  Window focused;
  int revert_to;
  XGetInputFocus(x11_display, &focused, &revert_to);
  return window->window == focused;
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
os_rect_from_window(OS_Handle handle)
{
  return os_client_rect_from_window(handle);
}

internal Rng2F32
os_client_rect_from_window(OS_Handle handle)
{
  X11_Window *window = x11_window_from_os_window(handle);

  XWindowAttributes attribs = {};
  XGetWindowAttributes(x11_display, window->window, &attribs);

  int x = attribs.x;
  int y = attribs.y;
  int width = attribs.width;
  int height = attribs.height;
  // Rng2F32 rect = r2f32(v2f32(x, y), v2f32(x + width, y + height));
  Rng2F32 rect = r2f32(v2f32(0, 0), v2f32(width, height));

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
  Vec2F32 v = v2f32(WidthOfScreen(x11_screen_ptr), HeightOfScreen(x11_screen_ptr));
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

// TODO:
// [ ] dpi, 
// [ ] monitor
// [ ] clipboard
// [ ] others?

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

      B32 release = 0;

      // Resize event
      switch (ev.type)
      {
      case Expose: 
      {
        if(window->repaint != 0)
        {
          window->repaint(os_window_from_x11_window(window), window->repaint_user_data);
        }
      } break;

      case KeyRelease:
      {
        release = 1;
      }
      case KeyPress:
      {
        B32 is_repeat = 0;
        if (ev.type == KeyRelease && XEventsQueued(x11_display, QueuedAfterReading))
        {
          XEvent nev;
          XPeekEvent(x11_display, &nev);

          if (nev.type == KeyPress && nev.xkey.time == ev.xkey.time &&
              nev.xkey.keycode == ev.xkey.keycode)
          {
            // key wasn’t actually released
            release = 0;
            is_repeat = 1;

            // eat next press event
            XNextEvent(x11_display, &nev);
          }
        }

        OS_Event *event = x11_push_event(release ? OS_EventKind_Release : OS_EventKind_Press, window);
        event->key = x11_scancode_to_os_key_table[ev.xkey.keycode & 0xFF];
        event->is_repeat = is_repeat;
        event->repeat_count = is_repeat;
        event->right_sided = false;
        // printf("Key: %s | Pressed: %d | Repeat: %d\n", os_g_key_display_string_table[event->key].str, !release, is_repeat);

        if(!release && window->input_context)
        {
          char buffer[100];
          Status status;
          int count = Xutf8LookupString(window->input_context, &ev.xkey, buffer, sizeof(buffer) - 1, NULL, &status);

          U8* data = (U8*)buffer;
          if(status == XBufferOverflow) 
          {
            data = push_array(arena, U8, count);
            count = Xutf8LookupString(window->input_context, &ev.xkey, buffer, sizeof(buffer) - 1, NULL, &status);
          }

          if(status == XLookupChars || status == XLookupBoth)
          {
            for(U64 offset = 0; offset < (U64)count;) 
            {
              UnicodeDecode dec = utf8_decode(data + offset, count - offset);
              offset += dec.inc;

              U32 character = dec.codepoint;
              if(character >= 32 && character != 127)
              {
                OS_Event *event = x11_push_event(OS_EventKind_Text, window);
                event->character = character;
                // printf("Codepoint: %u\n", event->character);
              }
            }
          }
        }
      } break;

      case ButtonRelease:
      {
        release = 1;
      }
      case ButtonPress:
      {
        #define Button6 6
        #define Button7 7

        // Mouse scroll events
        if(ev.type == ButtonPress &&
           (ev.xbutton.button == Button4 ||
            ev.xbutton.button == Button5 ||
            ev.xbutton.button == Button6 ||
            ev.xbutton.button == Button7)) 
        {
          OS_Event *event = x11_push_event(OS_EventKind_Scroll, window);
          Vec2F32 delta = {};
          switch(ev.xbutton.button) 
          {
            case Button4: { delta.y = -120.0f; }break;
            case Button5: { delta.y =  120.0f; }break;
            case Button6: { delta.x =  120.0f; }break;
            case Button7: { delta.x = -120.0f; }break;
          }
          event->pos.x = ev.xbutton.x;
          event->pos.y = ev.xbutton.y;
          event->delta = delta;
        } 
        else 
        {
          // Mouse button events
          OS_Event *event = x11_push_event(release ? OS_EventKind_Release : OS_EventKind_Press, window);
          switch (ev.xbutton.button)
          {
            case Button1: { event->key = OS_Key_LeftMouseButton;   }break;
            case Button2: { event->key = OS_Key_MiddleMouseButton; }break;
            case Button3: { event->key = OS_Key_RightMouseButton;  }break;
          }
          event->pos.x = ev.xbutton.x;
          event->pos.y = ev.xbutton.y;
        }
      } break;

      // Mouse movement
      case MotionNotify:
      {
        OS_Event *event = x11_push_event(OS_EventKind_MouseMove, window);
        event->pos.x = ev.xmotion.x;
        event->pos.y = ev.xmotion.y;
        // ("%f - %f\n", event->pos.x, event->pos.y);
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
  OS_EventFlags flags = 0;

  XkbStateRec state;
  XkbGetState(x11_display, XkbUseCoreKbd, &state);

  if(state.base_mods & ControlMask) flags |= OS_EventFlag_Ctrl;
  if(state.base_mods & ShiftMask)   flags |= OS_EventFlag_Shift;
  if(state.base_mods & Mod1Mask)    flags |= OS_EventFlag_Alt;

  return flags;
}

internal B32
os_key_is_down(OS_Key key)
{
  return 0;
}

internal Vec2F32
os_mouse_from_window(OS_Handle handle)
{
  X11_Window *window = x11_window_from_os_window(handle);
  
  Window root, child;
  int root_x, root_y, x, y;
  unsigned int mask;
  XQueryPointer(x11_display, window->window, &root, &child, &root_x, &root_y, &x, &y, &mask);
  return v2f32(x, y);
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

// TODO: move this
// dmylo: win32 OpenGL initialization stuff
// #if R_BACKEND == 2

internal void
os_init_opengl(R_OGL_Functions* gl_functions)
{
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
	Window window = XCreateWindow(x11_display, x11_root, 0, 0, 1, 1, 0, visual->depth, InputOutput, visual->visual, CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &window_attribs);

  glx_create_context_attribs_arb* glXCreateContextAttribsARB = (glx_create_context_attribs_arb*)glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );
	
	int context_attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
		GLX_CONTEXT_MINOR_VERSION_ARB, 1,
		GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		None
	};

	GLXContext context = glXCreateContextAttribsARB(x11_display, best_fbc, 0, true, context_attribs);
	XSync(x11_display, False);

	Bool result = glXMakeCurrent(x11_display, window, context);

  if(!result)
  {
    // TODO:error
    NotImplemented;
  }

  //-dmylo: Load OpenGL function and initialize state for the various passes
  {
    // dmylo: Load function pointers.
    // HMODULE opengl_module = LoadLibraryA("opengl32.dll");
    for (U64 i = 0; i < ArrayCount(r_ogl_g_function_names); i++)
    {
      void* ptr = (void*)glXGetProcAddressARB((GLubyte*)r_ogl_g_function_names[i]);
      // if(!ptr) {
      //   // dmylo: older functions are still in opengl32, so we also try that.
      //   ptr = GetProcAddress(opengl_module, r_ogl_g_function_names[i]);
      // }
      if(!ptr) {
        // dmylo: if still not found error out.
        char buffer[256] = {0};
        raddbg_snprintf(buffer, sizeof(buffer), "Failed to load OpenGL function: %s", r_ogl_g_function_names[i]);
        os_graphical_message(1, str8_lit("Fatal Error"), str8_cstring(buffer));
        os_exit_process(1);
      }
      gl_functions->_pointers[i] = ptr;
    }
  }

  x11_gl_context = context;
  x11_fbconfig = best_fbc;
  x11_dummy_window = window;
}

internal void
os_window_equip_opengl(OS_Handle handle)
{
}

internal void
os_window_unequip_opengl(OS_Handle handle)
{
}

internal void
os_window_begin_frame_opengl(OS_Handle handle)
{
  X11_Window *window = x11_window_from_os_window(handle);
	Bool result = glXMakeCurrent(x11_display, window->window, x11_gl_context);
}

internal void
os_window_end_frame_opengl(OS_Handle handle)
{
  X11_Window *window = x11_window_from_os_window(handle);
  glXSwapBuffers(x11_display, window->window);
}

// #endif
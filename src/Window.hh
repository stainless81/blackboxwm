// -*- mode: C++; indent-tabs-mode: nil; -*-
// Window.hh for Blackbox - an X11 Window manager
// Copyright (c) 2001 - 2002 Sean 'Shaleh' Perry <shaleh@debian.org>
// Copyright (c) 1997 - 2000 Brad Hughes (bhughes@tcac.net)
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef   __Window_hh
#define   __Window_hh

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#ifdef    SHAPE
#  include <X11/extensions/shape.h>
#endif // SHAPE
}

#include <string>

#include "BaseDisplay.hh"
#include "Timer.hh"
#include "Windowmenu.hh"

#define MwmHintsFunctions     (1l << 0)
#define MwmHintsDecorations   (1l << 1)

#define MwmFuncAll            (1l << 0)
#define MwmFuncResize         (1l << 1)
#define MwmFuncMove           (1l << 2)
#define MwmFuncIconify        (1l << 3)
#define MwmFuncMaximize       (1l << 4)
#define MwmFuncClose          (1l << 5)

#define MwmDecorAll           (1l << 0)
#define MwmDecorBorder        (1l << 1)
#define MwmDecorHandle        (1l << 2)
#define MwmDecorTitle         (1l << 3)
#define MwmDecorMenu          (1l << 4)
#define MwmDecorIconify       (1l << 5)
#define MwmDecorMaximize      (1l << 6)

// this structure only contains 3 elements... the Motif 2.0 structure contains
// 5... we only need the first 3... so that is all we will define
typedef struct MwmHints {
  unsigned long flags, functions, decorations;
} MwmHints;

#define PropMwmHintsElements  3


class BlackboxWindow : public TimeoutHandler {
private:
  Blackbox *blackbox;
  BScreen *screen;
  BTimer *timer;
  BlackboxAttributes blackbox_attrib;

  Time lastButtonPressTime;  // used for double clicks, when were we clicked
  Windowmenu *windowmenu;

  unsigned int window_number, workspace_number;
  unsigned long current_state;

  enum FocusMode { F_NoInput = 0, F_Passive,
                   F_LocallyActive, F_GloballyActive };
  FocusMode focus_mode;

  struct _flags {
    Bool moving,             // is moving?
      resizing,              // is resizing?
      shaded,                // is shaded?
      visible,               // is visible?
      iconic,                // is iconified?
      focused,               // has focus?
      stuck,                 // is omnipresent
      modal,                 // is modal? (must be dismissed to continue)
      send_focus_message,    // should we send focus messages to our client?
      shaped;                // does the frame use the shape extension?
    unsigned int maximized;  // maximize is special, the number corresponds
                             // with a mouse button
                             // if 0, not maximized
                             // 1 = HorizVert, 2 = Vertical, 3 = Horizontal
  } flags;

  struct _client {
    Window window,                  // the client's window
      window_group,                 // the client's window group
      transient_for;                // which window are we a transient for?
    BlackboxWindow *transient;      // which window is our transient?

    std::string title, icon_title;

    int x, y,
      old_bw;                       // client's borderwidth

    unsigned int width, height,
      title_text_w,                 // width as rendered in the current font
      min_width, min_height,        // can not be resized smaller
      max_width, max_height,        // can not be resized larger
      width_inc, height_inc,        // increment step
      min_aspect_x, min_aspect_y,   // minimum aspect ratio
      max_aspect_x, max_aspect_y,   // maximum aspect ratio
      base_width, base_height,
      win_gravity;

    unsigned long initial_state, normal_hint_flags, wm_hint_flags;
  } client;

  struct _functions {
    Bool resize, move, iconify, maximize, close;
  } functions;

  /*
   * client window = the application's window
   * frame window = the window drawn around the outside of the client window
   *                by the window manager which contains items like the
   *                titlebar and close button
   * title = the titlebar drawn above the client window, it displays the
   *         window's name and any buttons for interacting with the window,
   *         such as iconify, maximize, and close
   * label = the window in the titlebar where the title is drawn
   * buttons = maximize, iconify, close
   * handle = the bar drawn at the bottom of the window, which contains the
   *          left and right grips used for resizing the window
   * grips = the smaller reactangles in the handle, one of each side of it.
   *         When clicked and dragged, these resize the window interactively
   * border = the line drawn around the outside edge of the frame window,
   *          between the title, the bordered client window, and the handle.
   *          Also drawn between the grips and the handle
   */

  /*
   * what decorations do we have?
   * this is based on the type of the client window as well as user input
   * the menu is not really decor, but it goes hand in hand with the decor
   */
  struct _decorations {
    Bool titlebar, handle, border, iconify, maximize, close, menu;
  } decorations;

  struct _frame {
    // u -> unfocused, f -> has focus
    unsigned long ulabel_pixel, flabel_pixel, utitle_pixel,
      ftitle_pixel, uhandle_pixel, fhandle_pixel, ubutton_pixel,
      fbutton_pixel, pbutton_pixel, uborder_pixel, fborder_pixel,
      ugrip_pixel, fgrip_pixel;
    Pixmap ulabel, flabel, utitle, ftitle, uhandle, fhandle,
      ubutton, fbutton, pbutton, ugrip, fgrip;

    Window window,       // the frame
      plate,             // holds the client
      title,
      label,
      handle,
      close_button, iconify_button, maximize_button,
      right_grip, left_grip;

    /*
     * size and location of the box drawn while the window dimensions or
     * location is being changed, ie. resized or moved
     */
    int changing_x, changing_y;
    unsigned int changing_w, changing_h;

    int x, y,
      grab_x, grab_y,          // where was the window when it was grabbed?
      y_border, y_handle;      // where within frame is the border and handle

    unsigned int width, height, title_h, label_w, label_h, handle_h,
      button_w, button_h, grip_w, grip_h, mwm_border_w, border_h, border_w,
      bevel_w;
  } frame;

  BlackboxWindow(const BlackboxWindow&);
  BlackboxWindow& operator=(const BlackboxWindow&);

  Bool getState(void);
  Window createToplevelWindow(int x, int y, unsigned int width,
                              unsigned int height, unsigned int borderwidth);
  Window createChildWindow(Window parent, Cursor = None);

  void getWMName(void);
  void getWMIconName(void);
  void getWMNormalHints(void);
  void getWMProtocols(void);
  void getWMHints(void);
  void getMWMHints(void);
  Bool getBlackboxHints(void);
  void getTransientInfo(void);
  void setNetWMAttributes(void);
  void associateClientWindow(void);
  void decorate(void);
  void decorateLabel(void);
  void positionButtons(Bool redecorate_label = False);
  void positionWindows(void);
  void createHandle(void);
  void destroyHandle(void);
  void createTitlebar(void);
  void destroyTitlebar(void);
  void createCloseButton(void);
  void destroyCloseButton(void);
  void createIconifyButton(void);
  void destroyIconifyButton(void);
  void createMaximizeButton(void);
  void destroyMaximizeButton(void);
  void redrawLabel(void);
  void redrawAllButtons(void);
  void redrawCloseButton(Bool pressed);
  void redrawIconifyButton(Bool pressed);
  void redrawMaximizeButton(Bool pressed);
  void restoreGravity(void);
  void setGravityOffsets(void);
  void setState(unsigned long new_state);
  void upsize(void);
  void downsize(void);
  void right_fixsize(int *gx = 0, int *gy = 0);
  void left_fixsize(int *gx = 0, int *gy = 0);

public:
  BlackboxWindow(Blackbox *b, Window w, BScreen *s = (BScreen *) 0);
  virtual ~BlackboxWindow(void);

  inline Bool isTransient(void) const { return client.transient_for != None; }
  inline Bool isFocused(void) const { return flags.focused; }
  inline Bool isVisible(void) const { return flags.visible; }
  inline Bool isIconic(void) const { return flags.iconic; }
  inline Bool isShaded(void) const { return flags.shaded; }
  inline Bool isMaximized(void) const { return flags.maximized; }
  inline Bool isStuck(void) const { return flags.stuck; }
  inline Bool isIconifiable(void) const { return functions.iconify; }
  inline Bool isMaximizable(void) const { return functions.maximize; }
  inline Bool isResizable(void) const { return functions.resize; }
  inline Bool isClosable(void) const { return functions.close; }

  inline Bool hasTitlebar(void) const { return decorations.titlebar; }

  inline BlackboxWindow *getTransient(void) { return client.transient; }
  inline BlackboxWindow *getTransientFor(void)
  { return blackbox->searchWindow(client.transient_for); }

  inline BScreen *getScreen(void) { return screen; }

  inline Window getFrameWindow(void) const { return frame.window; }
  inline Window getClientWindow(void) const { return client.window; }

  inline Windowmenu * getWindowmenu(void) { return windowmenu; }

  inline const char *getTitle(void) const
  { return client.title.c_str(); }
  inline const char *getIconTitle(void) const
  { return client.icon_title.c_str(); }

  inline int getXFrame(void) const { return frame.x; }
  inline int getYFrame(void) const { return frame.y; }
  inline int getXClient(void) const { return client.x; }
  inline int getYClient(void) const { return client.y; }
  inline unsigned int getWorkspaceNumber(void) const
  { return workspace_number; }
  inline unsigned int getWindowNumber(void) const { return window_number; }

  inline unsigned int getWidth(void) const { return frame.width; }
  inline unsigned int getHeight(void) const { return frame.height; }
  inline unsigned int getClientHeight(void) const
  { return client.height; }
  inline unsigned int getClientWidth(void) const
  { return client.width; }
  inline unsigned int getTitleHeight(void) const
  { return frame.title_h; }

  inline void setWindowNumber(int n) { window_number = n; }

  Bool validateClient(void);
  Bool setInputFocus(void);

  void setFocusFlag(Bool focus);
  void iconify(void);
  void deiconify(Bool reassoc = True, Bool raise = True);
  void show(void);
  void close(void);
  void withdraw(void);
  void maximize(unsigned int button);
  void remaximize(void);
  void shade(void);
  void stick(void);
  void unstick(void);
  void reconfigure(void);
  void installColormap(Bool install);
  void restore(Bool remap);
  void configure(int dx, int dy, unsigned int dw, unsigned int dh);
  void setWorkspace(unsigned int n);
  void changeBlackboxHints(BlackboxHints *net);
  void restoreAttributes(void);

  void buttonPressEvent(XButtonEvent *be);
  void buttonReleaseEvent(XButtonEvent *re);
  void motionNotifyEvent(XMotionEvent *me);
  void destroyNotifyEvent(XDestroyWindowEvent */*unused*/);
  void mapRequestEvent(XMapRequestEvent *mre);
  void unmapNotifyEvent(XUnmapEvent */*unused*/);
  void reparentNotifyEvent(XReparentEvent */*unused*/);
  void propertyNotifyEvent(Atom atom);
  void exposeEvent(XExposeEvent *ee);
  void configureRequestEvent(XConfigureRequestEvent *cr);

#ifdef    SHAPE
  void configureShape(void);
  void shapeEvent(XShapeEvent * /*unused*/);
#endif // SHAPE

  virtual void timeout(void);
};


#endif // __Window_hh

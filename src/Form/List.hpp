/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009

	M Roberts (original release)
	Robin Birch <robinb@ruffnready.co.uk>
	Samuel Gisiger <samuel.gisiger@triadis.ch>
	Jeff Goodenough <jeff@enborne.f2s.com>
	Alastair Harrison <aharrison@magic.force9.co.uk>
	Scott Penrose <scottp@dd.com.au>
	John Wharington <jwharington@gmail.com>
	Lars H <lars_hn@hotmail.com>
	Rob Dunning <rob@raspberryridgesheepfarm.com>
	Russell King <rmk@arm.linux.org.uk>
	Paolo Ventafridda <coolwind@email.it>
	Tobias Lohner <tobias@lohner-net.de>
	Mirek Jezek <mjezek@ipplc.cz>
	Max Kellermann <max@duempel.org>
	Tobias Bieniek <tobias.bieniek@gmx.de>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#ifndef XCSOAR_FORM_LIST_HPP
#define XCSOAR_FORM_LIST_HPP

#include "Screen/PaintWindow.hpp"
#include "Form/ScrollBar.hpp"
#include "Compiler.h"

class ContainerWindow;

/**
 * A WndListFrame implements a scrollable list control based on the
 * WindowControl class.
 */
class WndListFrame : public PaintWindow {
  static const Color text_color;
  static const Color background_color;
  static const Color selected_background_color;

public:
  typedef void (*ActivateCallback_t)(unsigned idx);
  typedef void (*CursorCallback_t)(unsigned idx);
  typedef void (*PaintItemCallback_t)(Canvas &canvas, const RECT rc, unsigned idx);

protected:
  /** The ScrollBar object */
  ScrollBar scroll_bar;

  /** The height of one item on the screen, in pixels. */
  unsigned item_height;
  /** The number of items in the list. */
  unsigned length;
  /** The index of the topmost item currently being displayed. */
  unsigned origin;
  /** The number of items visible at a time. */
  unsigned items_visible;
  /** The index of the selected item on the screen. */
  unsigned relative_cursor;

  bool dragging;
  int drag_line;

  ActivateCallback_t ActivateCallback;
  CursorCallback_t CursorCallback;
  PaintItemCallback_t PaintItemCallback;

public:
  /**
   * Constructor of the WndListFrame class
   * @param parent the parent window
   * @param X x-Coordinate of the ListFrameControl
   * @param Y y-Coordinate of the ListFrameControl
   * @param Width Width of the ListFrameControl
   * @param Height Height of the ListFrameControl
   * @param _item_height Height of an item of the ListFrameControl
   */
  WndListFrame(ContainerWindow &parent,
               int X, int Y, int Width, int Height,
               const WindowStyle style,
               unsigned _item_height);

  /** Sets the function to call when a ListItem is chosen */
  void SetActivateCallback(ActivateCallback_t cb) {
    ActivateCallback = cb;
  }

  /** Sets the function to call when cursor has changed */
  void SetCursorCallback(CursorCallback_t cb) {
    CursorCallback = cb;
  }

  /** Sets the function to call when painting an item */
  void SetPaintItemCallback(PaintItemCallback_t cb) {
    PaintItemCallback = cb;
  }

  void SetItemHeight(unsigned _item_height);

  /**
   * Returns the number of items in the list
   * @return The number of items in the list
   */
  unsigned GetLength() const {
    return length;
  }

  /** Changes the number of items in the list. */
  void SetLength(unsigned n);

  /**
   * Returns the current cursor position
   * @return The current cursor position
   */
  unsigned GetCursorIndex() const {
    return origin + relative_cursor;
  }

  /**
   * Moves the cursor to the specified position.
   *
   * @return true if the cursor was moved to the specified position,
   * false if the position was invalid
   */
  bool SetCursorIndex(unsigned i);

  /**
   * Scrolls to the specified index.
   */
  void SetOrigin(unsigned i);

protected:
  /** Checks whether a ScrollBar is needed and shows/hides it */
  void show_or_hide_scroll_bar();

  /**
   * Scroll to the ListItem defined by i
   * @param i The ListItem array id
   */
  void EnsureVisible(unsigned i);

  /**
   * Determine which list item resides at the specified pixel row.
   * Returns -1 if there is no such list item.
   */
  gcc_pure
  int ItemIndexAt(int y) const {
    int i = y / item_height + origin;
    return i >= 0 && (unsigned)i < length ? i : -1;
  }

  void drag_end();

  /** Draws the ScrollBar */
  void DrawScrollBar(Canvas &canvas);

  /**
   * The on_resize event is called when the Control is resized
   * (derived from Window)
   */
  virtual bool on_resize(unsigned width, unsigned height);

  virtual bool on_setfocus();
  virtual bool on_killfocus();

  /**
   * The on_mouse_down event is called when the mouse is pressed over the button
   * (derived from Window)
   */
  virtual bool on_mouse_down(int x, int y);
  /**
   * The on_mouse_up event is called when the mouse is released over the button
   * (derived from Window)
   */
  virtual bool on_mouse_up(int x, int y);
  /**
   * The on_mouse_move event is called when the mouse is moved over the button
   * (derived from Window)
   */
  virtual bool on_mouse_move(int x, int y, unsigned keys);
  /**
   * The on_mouse_wheel event is called when the mouse wheel is turned
   * (derived from Window)
   */
  virtual bool on_mouse_wheel(int delta);

  virtual bool on_key_check(unsigned key_code);

  /**
   * The on_key_down event is called when a key is pressed while the
   * button is focused
   * (derived from Window)
   */
  virtual bool on_key_down(unsigned key_code);

  virtual bool on_cancel_mode();

  /**
   * The on_paint event is called when the button needs to be drawn
   * (derived from PaintWindow)
   */
  virtual void on_paint(Canvas &canvas);
};

#endif

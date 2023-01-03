#include "vterm_object.h"
#include "vterm.h"
#include <iostream>
#include <plog/Log.h>
#include <string.h>

int VTermObject::damage(VTermRect rect, void *user) {
  return ((VTermObject *)user)
      ->damage(rect.start_row, rect.start_col, rect.end_row, rect.end_col);
}

int VTermObject::moverect(VTermRect dest, VTermRect src, void *user) {
  return ((VTermObject *)user)->moverect(dest, src);
}

int VTermObject::movecursor(VTermPos pos, VTermPos oldpos, int visible,
                            void *user) {
  return ((VTermObject *)user)->movecursor(pos, oldpos, visible);
}

int VTermObject::settermprop(VTermProp prop, VTermValue *val, void *user) {
  return ((VTermObject *)user)->settermprop(prop, val);
}

int VTermObject::bell(void *user) { return ((VTermObject *)user)->bell(); }

int VTermObject::resize(int rows, int cols, void *user) {
  return ((VTermObject *)user)->resize(rows, cols);
}

int VTermObject::sb_pushline(int cols, const VTermScreenCell *cells,
                             void *user) {
  return ((VTermObject *)user)->sb_pushline(cols, cells);
}

int VTermObject::sb_popline(int cols, VTermScreenCell *cells, void *user) {
  return ((VTermObject *)user)->sb_popline(cols, cells);
}

VTermObject::VTermObject(int _rows, int _cols, VTermOutputCallback out,
                         void *user) {
  vterm_ = vterm_new(_rows, _cols);
  vterm_set_utf8(vterm_, 1);
  vterm_output_set_callback(vterm_, out, user);

  screen_ = vterm_obtain_screen(vterm_);
  vterm_screen_set_callbacks(screen_, &screen_callbacks, this);
  vterm_screen_reset(screen_, 1);
}

VTermObject::~VTermObject() { vterm_free(vterm_); }

void VTermObject::keyboard_unichar(char c, VTermModifier mod) {
  PLOG_DEBUG << c;
  vterm_keyboard_unichar(vterm_, c, mod);
}

void VTermObject::keyboard_key(VTermKey key, VTermModifier mod) {
  PLOG_DEBUG << key;
  vterm_keyboard_key(vterm_, key, mod);
}

void VTermObject::input_write(const char *bytes, size_t len) {
  vterm_input_write(vterm_, bytes, len);
}

const PosSet &VTermObject::new_frame(bool *ringing, bool check_damaed) {
  *ringing = ringing_;
  ringing_ = false;

  if (check_damaed) {
    std::swap(damaged_, tmp_);
  } else {
    tmp_.clear();
    int rows, cols;
    vterm_get_size(vterm_, &rows, &cols);
    for (int row = 0; row < rows; ++row) {
      for (int col = 0; col < cols; ++col) {
        tmp_.insert({.row = row, .col = col});
      }
    }
  }
  damaged_.clear();
  return tmp_;
}

VTermScreenCell *VTermObject::get_cell(VTermPos pos) const {
  // VTermScreenCell cell;
  vterm_screen_get_cell(screen_, pos, &cell_);
  if (cell_.chars[0] == 0xffffffff) {
    return nullptr;
  }
  if (VTERM_COLOR_IS_INDEXED(&cell_.fg)) {
    vterm_screen_convert_color_to_rgb(screen_, &cell_.fg);
  }
  if (VTERM_COLOR_IS_INDEXED(&cell_.bg)) {
    vterm_screen_convert_color_to_rgb(screen_, &cell_.bg);
  }
  return &cell_;
}

VTermScreenCell *VTermObject::get_cursor(VTermPos *pos) const {
  *pos = cursor_pos_;
  vterm_screen_get_cell(screen_, cursor_pos_, &cell_);
  return &cell_;
}

void VTermObject::set_rows_cols(int rows, int cols) {
  vterm_set_size(vterm_, rows, cols);
}

int VTermObject::damage(int start_row, int start_col, int end_row,
                        int end_col) {
  // PLOG_DEBUG << "damage: (" << start_row << ", " << start_col << ")-(" <<
  // end_row
  //           << "," << end_col << ")";
  for (int row = start_row; row < end_row; row++) {
    for (int col = start_col; col < end_col; col++) {
      damaged_.insert(VTermPos{
          .row = row,
          .col = col,
      });
    }
  }
  return 0;
}

int VTermObject::moverect(VTermRect dest, VTermRect src) {
  PLOG_DEBUG << "moverect";
  return 0;
}

int VTermObject::movecursor(VTermPos pos, VTermPos oldpos, int visible) {
  cursor_pos_ = pos;
  return 0;
}

int VTermObject::settermprop(VTermProp prop, VTermValue *val) {
  switch (prop) {
  case VTERM_PROP_CURSORVISIBLE:
    // bool
    PLOG_DEBUG << "VTERM_PROP_CURSORVISIBLE: " << val->boolean;
    break;
  case VTERM_PROP_CURSORBLINK:
    // bool
    PLOG_DEBUG << "VTERM_PROP_CURSORBLINK: " << val->boolean;
    break;
  case VTERM_PROP_ALTSCREEN:
    // bool
    PLOG_DEBUG << "VTERM_PROP_ALTSCREEN: " << val->boolean;
    break;
  case VTERM_PROP_TITLE:
    // string
    PLOG_DEBUG << "VTERM_PROP_TITLE: "
               << std::string_view(val->string.str, val->string.len);
    break;
  case VTERM_PROP_ICONNAME:
    // string
    PLOG_DEBUG << "VTERM_PROP_ICONNAME: "
               << std::string_view(val->string.str, val->string.len);
    break;
  case VTERM_PROP_REVERSE:
    // bool
    PLOG_DEBUG << "VTERM_PROP_REVERSE: " << val->boolean;
    break;
  case VTERM_PROP_CURSORSHAPE:
    // number
    PLOG_DEBUG << "VTERM_PROP_CURSORSHAPE: " << val->number;
    break;
  case VTERM_PROP_MOUSE:
    // number
    PLOG_DEBUG << "VTERM_PROP_MOUSE: " << val->number;
    break;
  default:
    PLOG_DEBUG << "unknown prop: " << prop;
  }
  return 0;
}

int VTermObject::bell() {
  PLOG_DEBUG << "bell";
  ringing_ = true;
  return 0;
}

int VTermObject::resize(int rows, int cols) {
  PLOG_DEBUG << "rows x cols: " << rows << " x " << cols;
  return 0;
}

int VTermObject::sb_pushline(int cols, const VTermScreenCell *cells) {
  PLOG_DEBUG << "sb_pushline";
  return 0;
}

int VTermObject::sb_popline(int cols, VTermScreenCell *cells) {
  PLOG_DEBUG << "sb_popline";
  return 0;
}

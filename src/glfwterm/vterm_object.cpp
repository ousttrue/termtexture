#include "vterm_object.h"
#include "vterm.h"
#include <iostream>
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
  std::cout << "resize: " << rows << ", " << cols << std::endl;
  return ((VTermObject *)user)->resize(rows, cols);
}

int VTermObject::sb_pushline(int cols, const VTermScreenCell *cells,
                             void *user) {
  return ((VTermObject *)user)->sb_pushline(cols, cells);
}

int VTermObject::sb_popline(int cols, VTermScreenCell *cells, void *user) {
  return ((VTermObject *)user)->sb_popline(cols, cells);
}

VTermObject::VTermObject(int _rows, int _cols, int font_width, int font_height,
                         VTermOutputCallback out, void *user) {
  vterm_ = vterm_new(_rows, _cols);
  vterm_set_utf8(vterm_, 1);
  vterm_output_set_callback(vterm_, out, user);

  screen_ = vterm_obtain_screen(vterm_);
  vterm_screen_set_callbacks(screen_, &screen_callbacks, this);
  vterm_screen_reset(screen_, 1);
}

VTermObject::~VTermObject() { vterm_free(vterm_); }

void VTermObject::keyboard_unichar(char c, VTermModifier mod) {
  vterm_keyboard_unichar(vterm_, c, mod);
}

void VTermObject::keyboard_key(VTermKey key, VTermModifier mod) {
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
  // std::cout << "damage: (" << start_row << ", " << start_col << ")-(" <<
  // end_row
  //           << "," << end_col << ")" << std::endl;
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
  std::cout << "moverect" << std::endl;
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
    std::cout << "VTERM_PROP_CURSORVISIBLE: " << val->boolean << std::endl;
    break;
  case VTERM_PROP_CURSORBLINK:
    // bool
    std::cout << "VTERM_PROP_CURSORBLINK: " << val->boolean << std::endl;
    break;
  case VTERM_PROP_ALTSCREEN:
    // bool
    std::cout << "VTERM_PROP_ALTSCREEN: " << val->boolean << std::endl;
    break;
  case VTERM_PROP_TITLE:
    // string
    std::cout << "VTERM_PROP_TITLE: "
              << std::string_view(val->string.str, val->string.len)
              << std::endl;
    break;
  case VTERM_PROP_ICONNAME:
    // string
    std::cout << "VTERM_PROP_ICONNAME: "
              << std::string_view(val->string.str, val->string.len)
              << std::endl;
    break;
  case VTERM_PROP_REVERSE:
    // bool
    std::cout << "VTERM_PROP_REVERSE: " << val->boolean << std::endl;
    break;
  case VTERM_PROP_CURSORSHAPE:
    // number
    std::cout << "VTERM_PROP_CURSORSHAPE: " << val->number << std::endl;
    break;
  case VTERM_PROP_MOUSE:
    // number
    std::cout << "VTERM_PROP_MOUSE: " << val->number << std::endl;
    break;
  default:
    std::cout << "unknown prop: " << prop << std::endl;
  }
  return 0;
}

int VTermObject::bell() {
  std::cout << "bell" << std::endl;
  ringing_ = true;
  return 0;
}

int VTermObject::resize(int rows, int cols) {
  std::cout << "resize" << std::endl;
  return 0;
}

int VTermObject::sb_pushline(int cols, const VTermScreenCell *cells) {
  std::cout << "sb_pushline" << std::endl;
  return 0;
}

int VTermObject::sb_popline(int cols, VTermScreenCell *cells) {
  std::cout << "sb_popline" << std::endl;
  return 0;
}

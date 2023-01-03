#pragma once
#include <functional>
#include <memory>
#include <stdexcept>
#include <unordered_set>
#include <vterm.h>
#include <optional>

template <> struct ::std::hash<VTermPos> {
  std::size_t operator()(const VTermPos &p) const noexcept {
    return p.row << 16 | p.col;
  }
};

struct PosEqual {
  constexpr bool operator()(VTermPos lhs, VTermPos rhs) const {
    return lhs.row == rhs.row && lhs.col == rhs.col;
  }
};

using PosSet = std::unordered_set<::VTermPos, std::hash<::VTermPos>, PosEqual>;

class VTermObject {
  VTerm *vterm_ = nullptr;
  VTermScreen *screen_ = nullptr;
  VTermPos cursor_pos_ = {};
  bool cursor_visible_ = true;
  mutable VTermScreenCell cell_ = {};
  bool ringing_ = false;

  PosSet damaged_;
  PosSet tmp_;

public:
  VTermObject(int _rows, int _cols, VTermOutputCallback out, void *user);
  ~VTermObject();
  void input_write(const char *bytes, size_t len);
  void keyboard_unichar(char c, VTermModifier mod);
  void keyboard_key(VTermKey key, VTermModifier mod);
  const PosSet &new_frame(bool *ringing, bool check_damaged = true);
  VTermScreenCell *get_cell(VTermPos pos) const;
  std::optional<VTermPos> get_cursor() const;
  void resize_rows_cols(int rows, int cols);

private:
  static int damage(VTermRect rect, void *user);
  static int moverect(VTermRect dest, VTermRect src, void *user);
  static int movecursor(VTermPos pos, VTermPos oldpos, int visible, void *user);
  static int settermprop(VTermProp prop, VTermValue *val, void *user);
  static int bell(void *user);
  static int resize(int rows, int cols, void *user);
  static int sb_pushline(int cols, const VTermScreenCell *cells, void *user);
  static int sb_popline(int cols, VTermScreenCell *cells, void *user);
  const VTermScreenCallbacks screen_callbacks = {
      damage, moverect, movecursor,  settermprop,
      bell,   resize,   sb_pushline, sb_popline};

  int damage(int start_row, int start_col, int end_row, int end_col);
  int moverect(VTermRect dest, VTermRect src);
  int movecursor(VTermPos pos, VTermPos oldpos, int visible);
  int settermprop(VTermProp prop, VTermValue *val);
  int bell();
  int resize(int rows, int cols);
  int sb_pushline(int cols, const VTermScreenCell *cells);
  int sb_popline(int cols, VTermScreenCell *cells);
};

#pragma once
#include <memory>
#include <vterm.h>

class Cursor {
  struct CursorImpl *impl_ = nullptr;

  Cursor();

public:
  ~Cursor();
  Cursor(const Cursor &) = delete;
  Cursor &operator=(const Cursor &) = delete;
  static std::shared_ptr<Cursor> Create();
  void Render(const VTermPos &pos);
};

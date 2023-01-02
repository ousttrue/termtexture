#pragma once
#include <span>

namespace common_pty {

class Pty {
  struct CommonPtyImpl *impl_ = nullptr;

public:
  Pty();
  ~Pty();
  void Launch(int rows, int cols, const char *cmd,
              const char *TERM = "xterm-256color");
  bool IsClosed();
  void Kill();
  void NotifyTermSize(unsigned short rows, unsigned short cols);
  void Write(const char *s, size_t len);
  std::span<char> Read();
};

} // namespace common_pty

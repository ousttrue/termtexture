#pragma once

namespace glo {

template <typename T> class ScopedBinder {
  const T &value_;

public:
  ScopedBinder(const ScopedBinder &) = delete;
  ScopedBinder &operator=(const ScopedBinder &) = delete;
  ScopedBinder(const T &value) : value_(value) { value_->Bind(); }
  ~ScopedBinder() { value_->Unbind(); }
};

template <typename T> static ScopedBinder<T> ScopedBind(const T &value) {
  return ScopedBinder<T>(value);
}

} // namespace glo

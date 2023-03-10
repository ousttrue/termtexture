#pragma once
#include <memory>
#include <stdint.h>

namespace glo {
class UBO {
  uint32_t ubo_;

  UBO();

public:
  UBO(const UBO &) = delete;
  UBO &operator=(const UBO &) = delete;
  ~UBO();
  static std::shared_ptr<UBO> Create();
  uint32_t Handle() { return ubo_; }
  void Bind();
  void Unbind();
  void Upload(const void *data, uint32_t size);
  template <typename T> void Upload(const T &t) { Upload(&t, sizeof(T)); }
  void BindBase(int binding);
};

template <typename T> struct TypedUBO {
  std::shared_ptr<UBO> ubo;
  T buffer;

  void Initialize() { ubo = UBO::Create(); }
  void Upload() { ubo->Upload(buffer); }
  uint32_t Handle() { return ubo->Handle(); }
};

} // namespace glo

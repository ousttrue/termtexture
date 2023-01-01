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
  void Bind();
  void Unbind();
  void Upload(const void *data, uint32_t size);
  void BindBase(int binding);
};

} // namespace glo

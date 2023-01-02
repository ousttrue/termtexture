#pragma once
#include <memory>
#include <span>
#include <stdint.h>

namespace glo {
class VBO {
  uint32_t vbo_ = 0;

  VBO();

public:
  VBO(const VBO &) = delete;
  VBO &operator=(const VBO &) = delete;
  ~VBO();
  static std::shared_ptr<VBO> Create();
  void SetData(uint32_t buffer_size, const void *data, bool is_dynamic = false);
  template <typename T>
  void DataFromSpan(std::span<T> data,
                                    bool is_dynamic = false) {
    SetData(data.size() * sizeof(T), data.data(), is_dynamic);
  }
  void SetSubData(const void *data, uint32_t offset, uint32_t size);
  template <typename T> void SubDataFromSpan(std::span<T> values) {
    SetSubData(values.data(), 0, sizeof(T) * values.size());
  }
  void Bind();
  void Unbind();
};

} // namespace glo

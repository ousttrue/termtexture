#pragma once
#include <fstream>
#include <string_view>
#include <vector>

namespace glo {

template <typename T>
static std::vector<T> ReadAllBytes(std::string_view path) {
  std::ifstream ifs(std::string(path).c_str(),
                    std::ios::binary | std::ios::ate);
  if (!ifs) {
    return {};
  }
  auto pos = ifs.tellg();
  auto size = pos / sizeof(T);
  if (pos % sizeof(T)) {
    ++size;
  }
  std::vector<T> buffer(size);
  ifs.seekg(0, std::ios::beg);
  ifs.read((char *)buffer.data(), pos);
  return buffer;
}

} // namespace glo

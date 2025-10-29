#pragma once

#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstring>
#include <concepts>
#include "header.hxx"

namespace bar::sys {
constexpr bool is_little_endian = (std::endian::native == std::endian::little);

template <std::integral T>
constexpr T bswap(T val) {
  auto* bytes = reinterpret_cast<uint8_t*>(&val);
  std::reverse(bytes, bytes + sizeof(T));
  return val;
}

template <typename T>
constexpr auto to_le(T val) -> T {
  if constexpr (is_little_endian) {
    return val;
  } else {
    return bswap(val);
  }
}

template <typename T>
constexpr auto from_le(T val) -> T {
  if constexpr (is_little_endian) {
    return val;
  } else {
    return bswap(val);
  }
}

void write_header(header::repr* buf, header_t header) {
  if constexpr (is_little_endian) {
    header.path = to_le(header.path);
    header.data = to_le(header.data);
    header.mode = to_le(header.mode);
    header.mtime = to_le(header.mtime);
  }
  std::memcpy(buf, &header, sizeof(header_t));
}

header_t read_header(const header::repr* buf) {
  header_t header;
  std::memcpy(&header, buf, sizeof(header_t));

  if constexpr (is_little_endian) {
    header.path = from_le(header.path);
    header.data = from_le(header.data);
    header.mode = from_le(header.mode);
    header.mtime = from_le(header.mtime);
  }
  return header;
}

}  // namespace bar::sys

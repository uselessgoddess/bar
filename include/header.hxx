#pragma once

#include <cstdint>
#include <array>

namespace bar {

constexpr std::array<uint8_t, 4> BAR = {0xf0, 0x9f, 0x8d, 0xbe};

enum struct entry_type : uint8_t { reg = 0, dir = 1, sym = 2 };

#pragma pack(push, 1)
struct header_t {
  entry_type type;
  uint8_t _____;  // reserved for 40 bytes
  int64_t mtime;
  uint16_t path;
  uint32_t mode;
  uint64_t data;
};  // 1 + 1 + 8 + 2 + 4 + 8
#pragma pack(pop)

static_assert(sizeof(header_t) == 24);  // less than cacheline

struct header {
  using repr = std::array<uint8_t, sizeof(header_t)>;
};

}  // namespace bar

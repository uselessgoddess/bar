#pragma once

#include <filesystem>
#include "header.hxx"

namespace fs = std::filesystem;

namespace bar {

class entry {
  fs::path path_;
  header_t header_;

 public:
  constexpr static auto MAX_NAME = std::numeric_limits<uint16_t>::max();

  entry(fs::path path, header_t header) : path_(std::move(path)), header_(header) {}

  auto& path() const { return path_; }
  fs::perms perms() const { return static_cast<fs::perms>(header_.mode); }

  uint64_t size() const { return header_.data; }

  bool is_dir() const { return header_.type == entry_type::dir; }
  bool is_reg() const { return header_.type == entry_type::reg; }

  header_t header() const { return header_; }
};

}  // namespace bar

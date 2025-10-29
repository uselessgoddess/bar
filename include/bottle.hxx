#pragma once

#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <iostream>  // temp
#include "header.hxx"
#include "entry.hxx"
#include "sys.hxx"

namespace fs = std::filesystem;
namespace chrono = std::chrono;
using ios = std::ios;

namespace bar {

namespace sys {
auto header_from(const fs::path& path) -> std::optional<header_t> {
  header_t header;
#if defined(__linux__) || defined(__APPLE__)
  struct stat st;
  if (lstat(path.c_str(), &st) != 0) {
    return std::nullopt;
  }
  header.mode = st.st_mode;
  header.mtime = st.st_mtime;
  if (S_ISDIR(st.st_mode)) {
    h.type = entry_type::dir;
  } else if (S_ISREG(st.st_mode)) {
    h.type = entry_type::reg;
    h.data = st.st_size;
  } else {
    // TODO !skip symlinks, etc.
    return std::nullopt;
  }
#else  // fallback
  auto status = fs::symlink_status(path);
  if (!fs::exists(status))
    return std::nullopt;

  auto mtime = chrono::duration_cast<chrono::seconds>(
      fs::last_write_time(path).time_since_epoch());
  header.mtime = mtime.count();
  header.mode = static_cast<uint32_t>(status.permissions());
  if (fs::is_directory(status))
    header.type = entry_type::dir;
  else if (fs::is_regular_file(status)) {
    header.type = entry_type::reg;
    header.data = fs::file_size(path);
  } else
    return std::nullopt;
#endif
  return header;
}
}  // namespace sys

struct bottle {
  explicit bottle(std::ostream& output) : output_(output) {
    output_.write(reinterpret_cast<const char*>(&BAR), sizeof(BAR));
  }

  void write_entry(header_t header, const fs::path& rel) {
    auto path_str = rel.generic_string();
    if (path_str.size() > entry::MAX_NAME)
      throw std::runtime_error("invalid path length (> u16::MAX)");

    header::repr buf;
    header.path = path_str.size();
    sys::write_header(&buf, header);

    output_.write(reinterpret_cast<const char*>(&buf), sizeof(buf));
    output_.write(path_str.data(), header.path);
    std::cout << "!" << path_str << "\n";
  }

  void write_entry(header_t header, const fs::path& rel, std::istream& input) {
    write_entry(header, rel);
    if (header.type == entry_type::reg && header.data > 0) {
      output_ << input.rdbuf();
    }
  }

  auto write_file(const fs::path& path) {
    if (auto header_opt = sys::header_from(path)) {
      std::ifstream in(path, ios::binary);
      write_entry(*header_opt, path.filename(), in);
    }
  }

  void write_dir_all(const fs::path& path) {
    auto base = path.parent_path();

    if (auto header_opt = sys::header_from(path)) {
      write_entry(*header_opt, fs::relative(path, base));
    }

    for (const auto& item : fs::recursive_directory_iterator(path)) {
      if (auto header_opt = sys::header_from(item.path())) {
        if (item.is_regular_file()) {
          std::ifstream in(item.path(), ios::binary);
          write_entry(*header_opt, fs::relative(item.path(), base), in);
        } else {
          write_entry(*header_opt, fs::relative(item.path(), base));
        }
      }
    }
  }

  std::ostream& output_;
};

}  // namespace bar

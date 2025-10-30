#pragma once

#include <cstdint>
#include <istream>
#include <cstring>
#include <vector>
#include "entry.hxx"
#include "sys.hxx"

namespace bar {

class opener {
  std::istream& input_;

 public:
  explicit opener(std::istream& in) : input_(in) {
    std::array<uint8_t, 4> magic;
    input_.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    if (input_.gcount() != sizeof(magic) || magic != BAR) {
      throw std::runtime_error("invalid bar archive: bad magic");
    }
  }

  auto next_entry() -> std::optional<entry> {
    header::repr buf;
    if (!input_.read(reinterpret_cast<char*>(&buf), sizeof(buf))) {
      return std::nullopt;
    }
    header_t header = sys::read_header(&buf);

    std::string path(header.path, '\0');
    if (!input_.read(path.data(), path.size())) {
      return std::nullopt;
    }
    return entry(fs::path(path), header);
  }

  void unpack(const entry& entry, const fs::path& path) {
    auto full_path = path / entry.path();
    if (entry.is_dir()) {
      fs::create_directories(full_path);
    } else if (entry.is_reg()) {
      fs::create_directories(full_path.parent_path());
      std::ofstream out(full_path, std::ios::binary);
      if (entry.size() > 0) {
        // TODO !use less suckless solution
        std::vector<char> buf(65536);
        for (uint64_t rem = entry.size(); rem > 0;) {
          input_.read(buf.data(), std::min<uint64_t>(rem, buf.size()));
          if (const auto n = input_.gcount()) {
            out.write(buf.data(), n);
            rem -= n;
          } else {
            break;
          }
        }
      }
    }

    fs::permissions(full_path, entry.perms());
  }

  void skip(const entry& entry) {
    if (entry.size() > 0) {
      input_.seekg(entry.size(), std::ios::cur);
    }
  }
};

}  // namespace bar

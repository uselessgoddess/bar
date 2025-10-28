#include <filesystem>
#include <limits>

#include "arc.hxx"

namespace fs = std::filesystem;
namespace chrono = std::chrono;

namespace bar {

class entry {
  fs::path path_;
  header_t header_;

 public:
  entry(fs::path path, header_t header) : path_(std::move(path)), header_(header) {}

  fs::perms perms() const { return static_cast<fs::perms>(header_.mode); }

  uint64_t size() const { return header_.data; }

  bool is_dir() const { return header_.type == entry_type::dir; }
  bool is_reg() const { return header_.type == entry_type::reg; }

  header_t header() const { return header_; }
};

constexpr auto MAX_NAME = std::numeric_limits<uint16_t>::max();

class bottle {
  std::ostream& output_;

 public:
  bottle(std::ostream& output) : output_(output) {
    output_.write(reinterpret_cast<const char*>(&BAR), sizeof(BAR));
  }

  auto write_file(const fs::path& full_path, const fs::path& rel_path) -> bool {
    std::string path_str = rel_path.generic_string();

    if (path_str.size() > MAX_NAME)
      return false;

    header_t header;

    header.path = path_str.size();
    header.data = 0;

#if defined(__linux__) || defined(__APPLE__)  // posix
    struct stat st;
    if (stat(full_path.c_str(), &st) != 0)
      return false;

    unalign::write(&header.) h.mode = st.st_mode;
    header.mtime = st.st_mtime;

    if (S_ISDIR(st.st_mode)) {
      header.type = entry_type::dir;
    } else if (S_ISREG(st.st_mode)) {
      header.type = entry_type::regular_file;
      header.data = st.st_size;
    } else {
      return false;  // todo! impl symlinks
    }
#else
    auto status = fs::symlink_status(full_path);
    header.mode = static_cast<uint32_t>(status.permissions());

    auto mtime = fs::last_write_time(full_path);
    header.mtime =
        chrono::duration_cast<chrono::seconds>(mtime.time_since_epoch()).count();

    if (fs::is_directory(status))
      header.type = entry_type::dir;
    else if (fs::is_regular_file(status)) {
      header.type = entry_type::reg;
      header.data = fs::file_size(full_path);
    } else
      return false;  // todo! impl symlinks
#endif

    output_.write(reinterpret_cast<const char*>(&header), sizeof(header));
    output_.write(path_str.data(), header.path);
    if (header.type == entry_type::reg && header.data > 0) {
      std::ifstream in(full_path, std::ios::binary);
      output_ << in.rdbuf();
    }

    return true;
  }

 public:
  void append(const fs::path& path) {
    const auto base = path.parent_path();

    if (fs::is_directory(path)) {
      write_file(path, fs::relative(path, base));
      for (const auto& entry : fs::recursive_directory_iterator(path)) {
        write_file(entry.path(), fs::relative(entry.path(), base));
      }
    } else {
      write_file(path, path.filename());
    }
  }
};

}  // namespace bar

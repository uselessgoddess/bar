#include <catch2/catch_test_macros.hpp>

#include <system_error>
#include <filesystem>
#include <fstream>
#include <bar.hxx>

namespace fs = std::filesystem;

namespace utils {
auto equals(const fs::path& pa, const fs::path& pb) -> bool {
  std::ifstream a(pa, std::ios::binary);
  std::ifstream b(pb, std::ios::binary);

  if (fs::file_size(pa) != fs::file_size(pb)) {
    return false;
  }

  return std::equal(std::istreambuf_iterator<char>(a.rdbuf()),
                    std::istreambuf_iterator<char>(),
                    std::istreambuf_iterator<char>(b.rdbuf()));
}
}  // namespace utils

TEST_CASE("Archive and extract round-trip", "[fs]") {
  auto ignore = std::error_code();

  auto temp_path = [&ignore](std::string_view path) {
    auto temp = fs::temp_directory_path() / path;
    fs::remove_all(temp, ignore);
    fs::remove(temp, ignore);
    return temp;
  };

  fs::path orig = temp_path("test");
  fs::path extr = temp_path("test.x");
  fs::path arch = temp_path("test.bar");

  fs::create_directories(orig / "subdir");
  std::ofstream(orig / "root.txt") << "root file";
  std::ofstream(orig / "subdir" / "nested.txt") << "nested file content";

  {
    std::ofstream out(arch, std::ios::binary);
    auto bottle = bar::bottle(out);
    bottle.write_dir_all(orig);
  }

  {
    std::ifstream in(arch, std::ios::binary);
    auto opener = bar::opener(in);
    while (auto entry_opt = opener.next_entry()) {
      opener.unpack(*entry_opt, extr);
    }
  }

  REQUIRE(fs::exists(extr / "test"));
  REQUIRE(fs::exists(extr / "test" / "root.txt"));
  REQUIRE(fs::exists(extr / "test" / "subdir" / "nested.txt"));

  REQUIRE(utils::equals(orig / "root.txt", extr / "test" / "root.txt"));
  REQUIRE(utils::equals(orig / "subdir" / "nested.txt",
                        extr / "test" / "subdir" / "nested.txt"));

  fs::remove_all(orig);
  fs::remove_all(extr);
  fs::remove(arch);
}

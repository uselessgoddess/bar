#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <fstream>
#include <filesystem>
#include <bar.hxx>

TEST_CASE("In-memory stream archive and metadata verification", "[stream]") {
  const std::string content = "✤⎋♡⣡◛⯵⿂➳⦥⟏⏍⑶⨞";

  std::stringstream input_stream(content);
  std::stringstream archive_stream;

  const fs::path path = "virtual/stream.txt";
  const fs::perms perms = fs::perms::owner_read | fs::perms::group_read;
  const auto mtime = chrono::system_clock::now().time_since_epoch().count();
  {
    bar::bottle b(archive_stream);
    b.write_entry(
        bar::header_t{
            .mtime = mtime,
            .mode = static_cast<uint32_t>(perms),
            .data = content.size(),
        },
        path, input_stream);
  }

  archive_stream.seekg(0);

  bar::opener op(archive_stream);
  auto entry_opt = op.next_entry();

  REQUIRE(entry_opt.has_value());
  const auto& entry = *entry_opt;

  CHECK(entry.path() == path);
  CHECK(entry.size() == content.size());
  CHECK(entry.perms() == perms);
  CHECK(entry.is_reg());
  CHECK_FALSE(entry.is_dir());

  CHECK_FALSE(op.next_entry().has_value());
}

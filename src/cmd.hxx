#pragma once

#include <cstdint>
#include <fstream>
#include <filesystem>
#include <bar.hxx>
#include <numeric>

#include "fmt.hxx"
#include "sad/sad.hxx"

namespace fs = std::filesystem;
namespace ranges = std::ranges;
using ios = std::ios;

struct config_t {
  bool sad = false;
};

auto size_of(const fs::path& path, uint64_t* files = nullptr) -> uint64_t {
  uint64_t size = 0;
  std::error_code ignore;

  if (fs::is_directory(path, ignore)) {
    for (const auto& item : fs::recursive_directory_iterator(path)) {
      if (item.is_regular_file(ignore)) {
        size += item.file_size(ignore);
        if (files != nullptr)
          *files += 1;
      }
    }
  } else if (fs::is_regular_file(path, ignore)) {
    size += fs::file_size(path, ignore);
    if (files != nullptr)
      *files += 1;
  }

  return size;
}

struct add_t {
  fs::path archive;
  std::vector<fs::path> inputs;
};

auto add(const config_t& config, const add_t& args) -> int {
  std::optional<ProgressBar> progress;

#if HAVE_BARKEEP
  if (!config.sad) {
    uint64_t files = 0;
    auto total_size = std::transform_reduce(
        args.inputs.begin(), args.inputs.end(), uint64_t(0), std::plus{},
        [&files](auto&& path) { return size_of(path, &files); });
    if (files > 1) {
      progress.emplace(files, total_size);
    }
  }
#endif

  std::ofstream out(args.archive, ios::binary);
  auto bottle = bar::bottle(out);

  for (const auto& path : args.inputs) {
    if (fs::is_directory(path)) {
      bottle.write_dir_all(path, [&progress](const fs::path& _, uint64_t size) {
        if (progress) {
          progress->inc(size);
        }
      });
    } else if (fs::is_regular_file(path)) {
      bottle.write_file(path);
    } else {
      fmt::eprintln("warning: skipping unsupported file type: {}", path.string());
    }
  }
  return EXIT_SUCCESS;
}

struct extract_t {
  fs::path archive;
  fs::path output;
};

auto extract(const extract_t& args) -> int {
  fs::create_directories(args.output);

  std::ifstream in(args.archive, ios::binary);
  bar::opener op(in);

  while (auto entry_opt = op.next_entry()) {
    op.unpack(*entry_opt, args.output);
  }

  return EXIT_SUCCESS;
}

struct list_t {
  fs::path archive;
};

auto list(const list_t& args) -> int {
  std::ifstream in(args.archive, ios::binary);
  auto opener = bar::opener(in);
  while (auto entry_opt = opener.next_entry()) {
    const auto& entry = *entry_opt;
    // TODO: !configurable color output
    fmt::println("{} {}\t\t{}", (entry.is_dir() ? 'd' : '-'), entry.size(),
                 entry.path().string());
    opener.skip(entry);
  }
  return EXIT_SUCCESS;
}

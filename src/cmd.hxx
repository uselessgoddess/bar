#pragma once

#include <fstream>
#include <filesystem>
#include <bar.hxx>

namespace fs = std::filesystem;
using ios = std::ios;

struct add_t {
  fs::path archive;
  std::vector<fs::path> inputs;
};

auto add(const add_t& args) -> int {
  std::ofstream out(args.archive, ios::binary);
  auto bottle = bar::bottle(out);

  for (const auto& path : args.inputs) {
    if (fs::is_directory(path)) {
      bottle.write_dir_all(path);
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

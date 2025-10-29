#include <cstdio>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <bar.hxx>
#include <format>
#include <ostream>
#include <utility>

#include "opener.hxx"
#include "utils/argh.h"

namespace fs = std::filesystem;
using ios = std::ios;

namespace std {
template <typename... Args>
inline void eprint(format_string<Args...> fmt, Args&&... args) {
  std::print(std::cerr, std::move(fmt), std::forward<decltype(args)...>(args...));
}
template <typename... Args>
inline void eprintln(format_string<Args...> fmt, Args&&... args) {
  std::println(std::cerr, std::move(fmt), std::forward<decltype(args)...>(args...));
}
}  // namespace std

constexpr auto USAGE =
    R"(usage: bar <command> [options] <archive> [files...]
    commands:
      a <files...> -o <archive>  Add files to archive
      x <archive> [-o <output>]  Extract files with full paths
      l <archive>                List contents of archive
    options:
     -o, --output <file>  Specify the output archive file
     -h, --help           Show this help message
    )";

auto add(const fs::path& archive_file, const std::vector<fs::path>& inputs) -> int {
  std::ofstream out(archive_file, ios::binary);
  auto bottle = bar::bottle(out);

  for (const auto& path : inputs) {
    if (fs::is_directory(path)) {
      bottle.write_dir_all(path);
    } else if (fs::is_regular_file(path)) {
      bottle.write_file(path);
    } else {
      std::cerr << "warning: skipping unsupported file type: " << path.string() << "\n";
    }
  }

  std::cout << "archive '" << archive_file.string() << "' created.\n";
  return EXIT_SUCCESS;
}

auto extract(const fs::path& archive_file, const fs::path& dest_dir) -> int {
  fs::create_directories(dest_dir);

  std::ifstream in(archive_file, ios::binary);
  bar::opener op(in);
  while (auto entry_opt = op.next_entry()) {
    op.unpack(*entry_opt, dest_dir);
  }

  return EXIT_SUCCESS;
}

auto list(const fs::path& archive_file) -> int {
  std::ifstream in(archive_file, ios::binary);
  auto opener = bar::opener(in);
  while (auto entry_opt = opener.next_entry()) {
    const auto& entry = *entry_opt;
    std::cout << std::format("{} {}\t\t{}\n", (entry.is_dir() ? 'd' : '-'), entry.size(),
                             entry.path().string());
    opener.skip(entry);
  }
  return EXIT_SUCCESS;
}

auto main(int argc, char* argv[]) -> int {
  std::ios_base::sync_with_stdio(false);

  argh::parser cmdl({"-o", "--output"});
  cmdl.parse(argc, argv);

  if (cmdl[{"-h", "--help"}]) {
    std::cout << USAGE;
    return EXIT_SUCCESS;
  }

  const auto& pos_args = cmdl.pos_args();
  if (pos_args.size() < 2) {
    std::cerr << "error: no command specified.\n" << USAGE;
    return EXIT_FAILURE;
  }

  const auto& command = pos_args[1];

  if (command == "a") {
    if (pos_args.size() < 3) {
      std::cerr << "error: 'a' command requires at least one input file.\n";
      return EXIT_FAILURE;
    }

    fs::path archive;
    if (!(cmdl({"-o", "--output"}) >> archive)) {
      std::cerr
          << "error: 'a' command requires an output archive file specified with -o.\n";
      return EXIT_FAILURE;
    }

    std::vector<fs::path> inputs;
    for (size_t i = 2; i < pos_args.size(); ++i) {
      inputs.emplace_back(pos_args[i]);
    }
    return add(archive, inputs);
  }

  if (command == "x") {
    if (pos_args.size() != 3) {
      std::cerr << "error: 'x' command requires exactly one archive name.\n";
      return EXIT_FAILURE;
    }
    fs::path archive = pos_args[2];
    fs::path output;
    cmdl({"-o", "--output"}) >> output;
    return extract(archive, output);
  }

  if (command == "l") {
    if (pos_args.size() != 3) {
      std::cerr << "error: 'l' command requires exactly one archive name.\n";
      return EXIT_FAILURE;
    }
    fs::path archive = pos_args[2];
    return list(archive);
  }

  std::cerr << "unknown command '" << command << "'.\n" << USAGE;
  return EXIT_FAILURE;
}

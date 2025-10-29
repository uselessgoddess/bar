#include <cstdio>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <bar.hxx>
#include <format>
#include <ostream>

#include "utils/argh.h"

namespace fs = std::filesystem;

constexpr auto USAGE =
    R"(usage: bar <command> [options] <archive> [files...]
    commands:
      a                Add files to archive
      x                Extract files with full paths
      l                List contents of archive
    options:
     -h, --help        Show this help message
    )";

auto add(const fs::path& archive_file, const std::vector<fs::path>& inputs) -> int {
  std::ofstream out(archive_file, std::ios::binary);
  bar::bottle b(out);

  for (const auto& input_path : inputs) {
    b.append(input_path);
  }

  std::cout << "archive '" << archive_file.string() << "' created.\n";
  return EXIT_SUCCESS;
}

auto extract(const fs::path& archive_file, const fs::path& dest_dir) -> int {
  fs::create_directories(dest_dir);

  std::ifstream in(archive_file, std::ios::binary);
  bar::opener op(in);
  while (auto entry_opt = op.next_entry()) {
    op.unpack(*entry_opt, dest_dir);
  }

  return EXIT_SUCCESS;
}

auto list(const fs::path& archive_file) -> int {
  std::ifstream in(archive_file, std::ios::binary);
  bar::opener op(in);
  while (auto entry_opt = op.next_entry()) {
    const auto& entry = *entry_opt;
    std::cout << std::format("{} {}\t\t{}\n", (entry.is_dir() ? 'd' : '-'), entry.size(),
                             entry.path().string());
    op.skip(entry);
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
    if (pos_args.size() < 4) {
      std::cerr << "add requires at least one file to add.\n";
      return EXIT_FAILURE;
    }
    fs::path archive_file = pos_args[2];
    std::vector<fs::path> inputs;
    for (size_t i = 3; i < pos_args.size(); ++i) {
      inputs.emplace_back(pos_args[i]);
    }
    return add(archive_file, inputs);
  }

  if (command == "x") {
    if (pos_args.size() != 3) {
      std::cerr << "extract requires only archive name.\n";
      return EXIT_FAILURE;
    }
    fs::path archive_file = pos_args[2];
    fs::path output;
    cmdl({"-o", "--output"}, ".") >> output;
    return extract(archive_file, output);
  }

  if (command == "l") {
    if (pos_args.size() != 3) {
      std::cerr << "list requries only archive name.\n";
      return EXIT_FAILURE;
    }
    fs::path archive_file = pos_args[2];
    return list(archive_file);
  }

  std::cerr << "unknown command '" << command << "'.\n" << USAGE;
  return EXIT_FAILURE;
}

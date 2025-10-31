#include <cstdio>
#include <functional>
#include <tuple>

#include <argparse/argparse.hpp>

#include <bar.hxx>
#include "fmt.hxx"
#include "cmd.hxx"

using argparse::ArgumentParser;

struct subcommand_t {
  ArgumentParser& parser;
  std::function<void(config_t)> handler;
};

auto add_cmd(ArgumentParser& cmd) {
  cmd.add_description("add files to an archive.");
  cmd.add_argument("-o", "--output").required().help("output archive file.");
  cmd.add_argument("files")
      .help("input files and directories.")
      .nargs(argparse::nargs_pattern::at_least_one);

  auto handler = [&cmd](config_t config) mutable {
    add_t args;
    args.archive = cmd.get<std::string>("-o");
    auto input_strings = cmd.get<std::vector<std::string>>("files");
    for (const auto& s : input_strings) {
      args.inputs.emplace_back(s);
    }
    add(config, args);
  };

  return subcommand_t(cmd, handler);
}

auto extract_cmd(ArgumentParser& cmd) {
  cmd.add_description("extract files from an archive.");
  cmd.add_argument("archive").help("archive file to extract.");
  cmd.add_argument("-o", "--output")
      .help("Directory to extract to.")
      .default_value(std::string("."));

  auto handler = [&cmd](config_t config) mutable {
    extract_t args;
    args.archive = cmd.get<std::string>("archive");
    args.output = cmd.get<std::string>("-o");
    extract(args);
  };

  return subcommand_t(cmd, handler);
}

auto list_cmd(ArgumentParser& cmd) {
  cmd.add_description("list contents of an archive.");
  cmd.add_argument("archive").help("archive file to list.");

  auto handler = [&cmd](config_t config) mutable {
    list_t args;
    args.archive = cmd.get<std::string>("archive");
    list(args);
  };

  return subcommand_t(cmd, handler);
}

auto main(int argc, char* argv[]) -> int {
  std::ios_base::sync_with_stdio(false);

  auto config = config_t{};

  ArgumentParser program("bar", "0.1");
  program.add_description("bar - bottle archiver.");
  program.add_argument("-s", "--sad")
      .help(fmt::format("suppress colors and animations (sad mode)."))
      .default_value(false)
      .implicit_value(true);

  ArgumentParser a("a"), x("x"), l("l");
  auto commands = std::make_tuple(add_cmd(a), extract_cmd(x), list_cmd(l));

  std::apply([&program](auto&&... cmds) { (program.add_subparser(cmds.parser), ...); },
             commands);

  try {
    program.parse_args(argc, argv);
  } catch (const std::runtime_error& err) {
    fmt::eprintln(emphasis::bold | fg(terminal_color::red), "{}", err.what());
    return EXIT_FAILURE;
  }

  try {
    std::apply(
        [&program, config](auto&&... cmds) {
          (
              ..., (void)[&program, config ](auto&& sub) {
                if (program.is_subcommand_used(sub.parser)) {
                  sub.handler(config);
                }
              }(cmds));
        },
        commands);
  } catch (const std::exception& e) {
    fmt::eprintln("error: {}", e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

#include <catch2/catch_session.hpp>
#include <catch2/catch_version_macros.hpp>

auto main(int argc, char* argv[]) -> int {
  auto session = Catch::Session();
  session.applyCommandLine(argc, argv);

  return session.run();
}

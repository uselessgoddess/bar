#include <cstdio>
#include <fstream>
#include <argh.h>
#include <bar.hxx>

auto main(int, char* argv[]) -> int {
  argh::parser args(argv);

  auto bar = std::ofstream("arc.bar", std::ios::binary);
  auto bottle = bar::bottle(bar);

  bottle.append("build");
}

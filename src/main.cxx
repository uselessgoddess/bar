#include <cstdio>
#include <argh.h>

auto main(int, char* argv[]) -> int {
  argh::parser args(argv);

  if (args[{"-h", "--help"}])
    puts("bottle archiver");
}

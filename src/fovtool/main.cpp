
#include <stdlib.h>

#include <CLI/CLI.hpp>
#include <clocale>

int main(int argc, char** argv) {
  std::setlocale(LC_ALL, ".UTF-8");

  CLI::App app{"Compute field-of-view tool"};

  CLI11_PARSE(app, argc, argv);
  return EXIT_SUCCESS;
}


// Silence warnings for fmt. Update fmt and check if this isn't needed on releases after 11.0.2
#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING
#include <fmt/format.h>
#undef _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING

#include <stdlib.h>
#include <utf8.h>

#include <CLI/CLI.hpp>
#include <clocale>
#include <filesystem>
#include <gsl/gsl>
#include <iostream>
#include <locale>
#include <ranges>
#include <sstream>
#include <string>

#include "libtcod-fov.h"

struct MapInfo {
  tcod::fov::TCODMap data;  // Map tile data
  std::vector<std::tuple<int, int>> sources{};  // POV/light sources
};

static auto load_map(const std::filesystem::path& path) {
  auto lines = std::vector<std::u32string>{};
  {
    auto in_file = std::ifstream{path};
    auto line_number = gsl::index{1};
    for (std::string line; std::getline(in_file, line); ++line_number) {
      if (utf8::find_invalid(line.begin(), line.end()) != line.end()) {
        throw std::runtime_error{fmt::format("Invalid UTF-8 encoding detected on line {}", line_number)};
      }
      lines.emplace_back(utf8::utf8to32(line));
    }
  }
  while (lines.size() && lines.at(lines.size() - 1).size() == 0) lines.pop_back();
  const int map_height = gsl::narrow<int>(lines.size());
  const int map_width =
      std::ranges::max(lines | std::views::transform([](const auto& line) { return gsl::narrow<int>(line.size()); }));
  auto map = MapInfo{
      .data = {map_width, map_height},
  };
  for (int y = 0; y < gsl::narrow<int>(lines.size()); ++y) {
    const auto& line = lines.at(y);
    for (int x = 0; x < map_width; ++x) {
      static constexpr auto DEFAULT_CH = '.';
      const auto ch = (x < gsl::narrow<int>(line.size()) ? line.at(x) : DEFAULT_CH);
      const bool transparent = ch != '#';
      map.data.setProperties(x, y, transparent, false);
      if (ch == '@') {
        map.sources.push_back({x, y});
      }
    }
  }
  return map;
}

static auto render_map(const MapInfo& map) {
  auto stream = std::ostringstream{};
  for (int y = 0; y < map.data.getHeight(); ++y) {
    if (y) stream << '\n';
    for (int x = 0; x < map.data.getWidth(); ++x) {
      if (std::ranges::any_of(map.sources, [=](const auto& xy) { return xy == std::tuple<int, int>{x, y}; })) {
        stream << '@';
        continue;
      }
      const bool visible = map.data.isInFov(x, y);
      const bool transparent = map.data.isTransparent(x, y);
      stream << (visible ? (transparent ? '.' : '#') : (transparent ? ' ' : ' '));
    }
  }
  return stream.rdbuf()->str();
}

int main(int argc, char** argv) {
  std::setlocale(LC_ALL, ".UTF-8");

  auto app = CLI::App{"Compute field-of-view tool"};

  auto input_str = std::string{};
  app.add_option("-i,--input", input_str, "The input file, should be a UTF8 text file")
      ->check(CLI::ExistingFile)
      ->required();

  auto algorithm = std::string{};
  app.add_option("-a,--algo", algorithm, "The FOV algorithm to invoke");

  CLI11_PARSE(app, argc, argv);

  try {
    auto map = load_map(input_str);
    for (const auto& [x, y] : map.sources) {
      TCODFOV_map_compute_fov(map.data.data, x, y, 0, true, TCODFOV_SYMMETRIC_SHADOWCAST);
      std::cout << render_map(map) << '\n';
    }
  } catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << '\n';
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

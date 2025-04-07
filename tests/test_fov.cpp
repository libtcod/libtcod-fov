
#include <algorithm>
#include <catch2/catch_all.hpp>
#include <filesystem>
#include <fstream>
#include <gsl/gsl>
#include <iostream>
#include <memory>
#include <random>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

#include "libtcod-fov/fov.hpp"
#include "libtcod-fov/libtcod_int.h"
#include "libtcod-fov/map.hpp"

struct MapInfo {
  std::string name{};
  tcod::fov::Bitpacked2D data{};  // Map tile data
  std::tuple<int, int> pov{0, 0};
};

/// @brief Load ASCII maps from files
static auto load_map(std::string name, const std::filesystem::path& path) {
  auto lines = std::vector<std::string>{};
  {
    auto in_file = std::ifstream{path};
    for (std::string line; std::getline(in_file, line);) {
      lines.emplace_back((line));
    }
  }
  while (lines.size() && lines.at(lines.size() - 1).size() == 0) lines.pop_back();
  const int map_height = gsl::narrow<int>(lines.size());
  const int map_width =
      std::ranges::max(lines | std::views::transform([](const auto& line) { return gsl::narrow<int>(line.size()); }));
  auto map = MapInfo{
      .name = std::move(name),
      .data = tcod::fov::Bitpacked2D{{map_height, map_width}},
  };
  for (int y = 0; y < gsl::narrow<int>(lines.size()); ++y) {
    const auto& line = lines.at(y);
    for (int x = 0; x < map_width; ++x) {
      static constexpr auto DEFAULT_CH = '.';
      const auto ch = (x < gsl::narrow<int>(line.size()) ? line.at(x) : DEFAULT_CH);
      const bool transparent = ch != '#';
      map.data.set_bool({y, x}, transparent);
      if (ch == '@') {
        map.pov = {x, y};
      }
    }
  }
  return map;
}

static auto new_map_with_radius(int radius, bool start_transparent) -> tcod::fov::Bitpacked2D {
  const int size = radius * 2 + 1;
  tcod::fov::Bitpacked2D map{{size, size}, start_transparent};
  return map;
}
static auto new_empty_map(int radius) -> tcod::fov::Bitpacked2D { return new_map_with_radius(radius, true); }
static auto new_opaque_map(int radius) -> tcod::fov::Bitpacked2D { return new_map_with_radius(radius, false); }
static auto new_corridor_map(int radius) -> tcod::fov::Bitpacked2D {
  tcod::fov::Bitpacked2D map{new_map_with_radius(radius, false)};
  for (int i = 0; i < radius * 2 + 1; ++i) {
    map.set_bool({i, radius}, true);
    map.set_bool({radius, i}, true);
  }
  return map;
}
static auto new_forest_map(int radius) -> tcod::fov::Bitpacked2D {
  // Forest map with 1 in 4 chance of a blocking tile.
  std::mt19937 rng(0);
  std::uniform_int_distribution<int> chance(0, 3);
  tcod::fov::Bitpacked2D map{new_map_with_radius(radius, true)};
  for (auto& it : map) it = (chance(rng) == 0);
  return map;
}

static auto center_of_radius(int radius) -> std::tuple<int, int> { return {radius, radius}; }

TEST_CASE("FOV Benchmarks", "[.benchmark]") {
  std::array test_maps{
      load_map("pillars", "data/pillars.txt"),
      MapInfo{"empty_r4", new_empty_map(4), center_of_radius(4)},
      MapInfo{"empty_r10", new_empty_map(10), center_of_radius(10)},
      MapInfo{"empty_r50", new_empty_map(50), center_of_radius(50)},
      MapInfo{"empty_r300", new_empty_map(300), center_of_radius(300)},
      MapInfo{"opaque_r4", new_opaque_map(4), center_of_radius(4)},
      MapInfo{"opaque_r10", new_opaque_map(10), center_of_radius(10)},
      MapInfo{"opaque_r50", new_opaque_map(50), center_of_radius(50)},
      MapInfo{"opaque_r300", new_opaque_map(300), center_of_radius(300)},
      MapInfo{"corridor_r4", new_corridor_map(4), center_of_radius(4)},
      MapInfo{"corridor_r10", new_corridor_map(10), center_of_radius(10)},
      MapInfo{"corridor_r50", new_corridor_map(50), center_of_radius(50)},
      MapInfo{"corridor_r300", new_corridor_map(300), center_of_radius(300)},
      MapInfo{"forest_r4", new_forest_map(4), center_of_radius(4)},
      MapInfo{"forest_r10", new_forest_map(10), center_of_radius(10)},
      MapInfo{"forest_r50", new_forest_map(50), center_of_radius(50)},
      MapInfo{"forest_r300", new_forest_map(300), center_of_radius(300)},
  };
  for (auto& active_test : test_maps) {
    const std::string& map_name = active_test.name;
    const auto& map = active_test.data;
    auto out = tcod::fov::Bitpacked2D{map.get_shape()};
    const auto [pov_x, pov_y] = active_test.pov;
    BENCHMARK(map_name + " TCODFOV_BASIC") {
      (void)!TCODFOV_map_compute_fov_circular_raycasting(map.get_ptr(), out.get_ptr(), pov_x, pov_y, 0, true);
    };
    BENCHMARK(map_name + " TCODFOV_DIAMOND") {
      (void)!TCODFOV_map_compute_fov_diamond_raycasting(map.get_ptr(), out.get_ptr(), pov_x, pov_y, 0, true);
    };
    BENCHMARK(map_name + " TCODFOV_SHADOW") {
      (void)!TCODFOV_map_compute_fov_recursive_shadowcasting(map.get_ptr(), out.get_ptr(), pov_x, pov_y, 0, true);
    };
    BENCHMARK(map_name + " TCODFOV_RESTRICTIVE") {
      (void)!TCODFOV_map_compute_fov_restrictive_shadowcasting(map.get_ptr(), out.get_ptr(), pov_x, pov_y, 0, true);
    };
    BENCHMARK(map_name + " TCODFOV_PERMISSIVE_8") {
      (void)!TCODFOV_map_compute_fov_permissive2(map.get_ptr(), out.get_ptr(), pov_x, pov_y, 0, true, 8);
    };
    BENCHMARK(map_name + " TCODFOV_SYMMETRIC_SHADOWCAST") {
      (void)!TCODFOV_map_compute_fov_symmetric_shadowcast(map.get_ptr(), out.get_ptr(), pov_x, pov_y, 0, true);
    };
  }
}

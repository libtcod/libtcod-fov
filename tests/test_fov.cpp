
#include <algorithm>
#include <catch2/catch_all.hpp>
#include <filesystem>
#include <fstream>
#include <gsl/gsl>
#include <iostream>
#include <libtcod-fov/fov.hpp>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

struct MapInfo {
  std::string name{};
  tcod::fov::MapPtr_ data{};  // Map tile data
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
      .data = tcod::fov::MapPtr_{TCODFOV_map_new(map_width, map_height)},
  };
  for (int y = 0; y < gsl::narrow<int>(lines.size()); ++y) {
    const auto& line = lines.at(y);
    for (int x = 0; x < map_width; ++x) {
      static constexpr auto DEFAULT_CH = '.';
      const auto ch = (x < gsl::narrow<int>(line.size()) ? line.at(x) : DEFAULT_CH);
      const bool transparent = ch != '#';
      TCODFOV_map_set_properties(map.data.get(), x, y, transparent, false);
      if (ch == '@') {
        map.pov = {x, y};
      }
    }
  }
  return map;
}

static auto new_map_with_radius(int radius, bool start_transparent) -> tcod::fov::MapPtr_ {
  const int size = radius * 2 + 1;
  tcod::fov::MapPtr_ map{TCODFOV_map_new(size, size)};
  TCODFOV_map_clear(map.get(), start_transparent, 0);
  return map;
}
static auto new_empty_map(int radius) -> tcod::fov::MapPtr_ { return new_map_with_radius(radius, true); }
static auto new_opaque_map(int radius) -> tcod::fov::MapPtr_ { return new_map_with_radius(radius, false); }
static auto new_corridor_map(int radius) -> tcod::fov::MapPtr_ {
  tcod::fov::MapPtr_ map{new_map_with_radius(radius, false)};
  for (int i = 0; i < radius * 2 + 1; ++i) {
    TCODFOV_map_set_properties(map.get(), radius, i, true, true);
    TCODFOV_map_set_properties(map.get(), i, radius, true, true);
  }
  return map;
}
static auto new_forest_map(int radius) -> tcod::fov::MapPtr_ {
  // Forest map with 1 in 4 chance of a blocking tile.
  std::mt19937 rng(0);
  std::uniform_int_distribution<int> chance(0, 3);
  tcod::fov::MapPtr_ map{new_map_with_radius(radius, true)};
  for (int i = 0; i < map->nbcells; ++i) {
    if (chance(rng) == 0) {
      map->cells[i].transparent = false;
    }
  }
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
    TCODFOV_Map* map = active_test.data.get();
    const auto [pov_x, pov_y] = active_test.pov;
    BENCHMARK(map_name + " TCODFOV_BASIC") {
      (void)!TCODFOV_map_compute_fov(map, pov_x, pov_y, 0, true, TCODFOV_BASIC);
    };
    BENCHMARK(map_name + " TCODFOV_DIAMOND") {
      (void)!TCODFOV_map_compute_fov(map, pov_x, pov_y, 0, true, TCODFOV_DIAMOND);
    };
    BENCHMARK(map_name + " TCODFOV_SHADOW") {
      (void)!TCODFOV_map_compute_fov(map, pov_x, pov_y, 0, true, TCODFOV_SHADOW);
    };
    BENCHMARK(map_name + " TCODFOV_RESTRICTIVE") {
      (void)!TCODFOV_map_compute_fov(map, pov_x, pov_y, 0, true, TCODFOV_RESTRICTIVE);
    };
    BENCHMARK(map_name + " TCODFOV_PERMISSIVE_8") {
      (void)!TCODFOV_map_compute_fov(map, pov_x, pov_y, 0, true, TCODFOV_PERMISSIVE_8);
    };
    BENCHMARK(map_name + " TCODFOV_SYMMETRIC_SHADOWCAST") {
      (void)!TCODFOV_map_compute_fov(map, pov_x, pov_y, 0, true, TCODFOV_SYMMETRIC_SHADOWCAST);
    };
  }
}

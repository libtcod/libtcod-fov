
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

// Regression for the view_array_insert off-by-one: it writes one past the
// active_views buffer (sized width*height). The only trigger is a 2x2 map at
// FOV_PERMISSIVE_8; under AddressSanitizer this aborts before the fix.
TEST_CASE("permissive FOV does not overflow active_views on tiny maps", "[fov]") {
  for (int w = 1; w <= 3; ++w)
    for (int h = 1; h <= 3; ++h)
      for (int pov = 0; pov < w * h; ++pov) {
        TCODFOV_Map* map = TCODFOV_map_new(w, h);
        TCODFOV_map_clear(map, false, true);  // every cell opaque
        CHECK(TCODFOV_map_compute_fov(map, pov % w, pov / w, 0, true, TCODFOV_PERMISSIVE_8) == TCODFOV_E_OK);
        TCODFOV_map_delete(map);
      }
}

// Regression for the undersized per-quadrant `bumps` buffer (sized width*height).
// Reproduced on 2x3 / 3x2 maps, which never reach the active-view count that
// triggers bug #1, so this isolates the bumps overflow. ASan aborts pre-fix.
TEST_CASE("permissive FOV does not overflow bumps on tiny maps", "[fov]") {
  const int dims[][2] = {{2, 3}, {3, 2}};
  for (const auto& d : dims) {
    const int n = d[0] * d[1];
    for (int pattern = 0; pattern < (1 << n); ++pattern)
      for (int pov = 0; pov < n; ++pov) {
        if (pattern & (1 << pov)) continue;
        for (int light_walls = 0; light_walls <= 1; ++light_walls) {
          TCODFOV_Map* map = TCODFOV_map_new(d[0], d[1]);
          TCODFOV_map_clear(map, true, true);
          for (int i = 0; i < n; ++i)
            if (pattern & (1 << i)) TCODFOV_map_set_properties(map, i % d[0], i / d[0], false, true);
          CHECK(
              TCODFOV_map_compute_fov(map, pov % d[0], pov / d[0], 0, (bool)light_walls, TCODFOV_PERMISSIVE_8) ==
              TCODFOV_E_OK);
          TCODFOV_map_delete(map);
        }
      }
  }
}

// Regression for the undersized obstacle buffer in MRPAS restrictive FOV
// (TCOD_map_compute_fov_restrictive_shadowcasting). Its angle buffers are sized
// `max_obstacles = nbcells / 7`, which floors below the true per-quadrant
// obstacle count on small maps; the scan then writes one obstacle past the end
// of start_angle / end_angle -- always a single 8-byte (one double) overrun.
// Unpatched, AddressSanitizer aborts at fov_restrictive.c compute_quadrant.

// Part 1 -- exhaustive sweep of small maps: every wall pattern x every POV x
// both light_walls settings. Each of these shapes overflows pre-fix (peak
// obstacles = nbcells/7 + 1). Includes non-square shapes (2x6, 3x4) to show the
// bug is not limited to squares.
TEST_CASE("restrictive FOV: small-map obstacle overflow (exhaustive)", "[fov]") {
  const int dims[][2] = {{2, 2}, {2, 3}, {3, 2}, {3, 3}, {2, 6}, {3, 4}, {4, 4}};
  for (const auto& d : dims) {
    const int w = d[0], h = d[1], n = w * h;
    for (long pattern = 0; pattern < (1L << n); ++pattern)
      for (int pov = 0; pov < n; ++pov) {
        if (pattern & (1L << pov)) continue;
        for (int light_walls = 0; light_walls <= 1; ++light_walls) {
          TCODFOV_Map* map = TCODFOV_map_new(w, h);
          TCODFOV_map_clear(map, true, true);
          for (int i = 0; i < n; ++i)
            if (pattern & (1L << i)) TCODFOV_map_set_properties(map, i % w, i / w, false, true);
          CHECK(
              TCODFOV_map_compute_fov(map, pov % w, pov / w, 0, (bool)light_walls, TCODFOV_RESTRICTIVE) ==
              TCODFOV_E_OK);
          TCODFOV_map_delete(map);
        }
      }
  }
}

// Part 2 -- the overflow envelope is shape-dependent, not a 5x5 box. The peak
// obstacle count saturates with the map's smaller dimension while nbcells/7
// grows with area, so each width has a band of heights that overflow. Exhaustive
// search is infeasible past ~16 cells (2^25+ patterns), so these are concrete
// adversarial layouts -- the largest overflowing shape per width -- found by a
// peak-obstacle search and verified to overflow the unpatched library. The
// largest is 4x8 (32 cells): a 5th obstacle written into a 4-slot (32-byte)
// buffer. All use light_walls=true and an unbounded radius (0).
namespace {
struct Layout {
  const char* name;
  int w, h, pov_x, pov_y;
  std::vector<std::pair<int, int>> walls;
};
}  // namespace

TEST_CASE("restrictive FOV: larger non-square overflow envelope", "[fov][bug3]") {
  const std::vector<Layout> layouts = {
      {"5x5", 5, 5, 0, 4, {{0, 0}, {3, 0}, {4, 0}, {1, 1}, {2, 1}, {4, 1}, {1, 2}, {4, 2}, {0, 3}, {4, 3}}},
      {"3x8", 3, 8, 2, 0, {{1, 0}, {0, 1}, {0, 2}, {0, 4}, {0, 7}, {1, 7}}},
      {"4x6", 4, 6, 3, 0, {{0, 0}, {0, 1}, {1, 1}, {1, 2}, {0, 5}, {1, 5}, {2, 5}}},
      {"3x9", 3, 9, 2, 7, {{0, 0}, {1, 0}, {0, 3}, {0, 5}, {0, 7}, {1, 7}, {2, 8}}},
      {"4x8", 4, 8, 0, 7, {{1, 0}, {2, 0}, {3, 0}, {3, 2}, {3, 4}, {3, 6}, {2, 7}}},
  };
  for (const auto& L : layouts) {
    CAPTURE(L.name);
    TCODFOV_Map* map = TCODFOV_map_new(L.w, L.h);
    TCODFOV_map_clear(map, true, true);
    for (const auto& wall : L.walls) TCODFOV_map_set_properties(map, wall.first, wall.second, false, true);
    CHECK(TCODFOV_map_compute_fov(map, L.pov_x, L.pov_y, 0, true, TCODFOV_RESTRICTIVE) == TCODFOV_E_OK);
    TCODFOV_map_delete(map);
  }
}

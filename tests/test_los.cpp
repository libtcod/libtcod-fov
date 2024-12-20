
#include <algorithm>
#include <array>
#include <catch2/catch_all.hpp>
#include <vector>

#include "libtcod-fov/bresenham.hpp"
#include "libtcod-fov/dda.h"

struct Point2D {
  int x;
  int y;

  bool operator==(const Point2D& other) const noexcept { return x == other.x && y == other.y; }

  friend std::ostream& operator<<(std::ostream& out, const Point2D& data) {
    return out << '{' << data.x << ',' << ' ' << data.y << '}';
  }
};

/**
    Return a vector of bresenham coordinates, including both endpoints.
 */
std::vector<std::array<int, 2>> generate_line(const std::array<int, 2>& begin, const std::array<int, 2>& end) {
  TCODFOV_bresenham_data_t bresenham_stack_data;
  std::vector<std::array<int, 2>> line;
  int x = begin.at(0);
  int y = begin.at(1);
  TCODFOV_line_init_mt(begin.at(0), begin.at(1), end.at(0), end.at(1), &bresenham_stack_data);
  line.push_back(begin);
  while (!TCODFOV_line_step_mt(&x, &y, &bresenham_stack_data)) {
    line.push_back({x, y});
  }
  return line;
}

/// Return a vector of DDA coordinates, including both endpoints.
std::vector<Point2D> generate_line_dda(const std::array<int, 2>& begin, const std::array<int, 2>& end) {
  auto length = TCODFOV_dda_compute(begin.at(0), begin.at(1), end.at(0), end.at(1), 0, nullptr);
  std::vector<Point2D> line(length);
  TCODFOV_dda_compute(begin.at(0), begin.at(1), end.at(0), end.at(1), length, reinterpret_cast<int*>(line.data()));
  return line;
}

std::vector<Point2D> generate_line_dda_orthogonal(const std::array<int, 2>& begin, const std::array<int, 2>& end) {
  auto length = TCODFOV_dda_compute_orthogonal(begin.at(0), begin.at(1), end.at(0), end.at(1), 0, nullptr);
  std::vector<Point2D> line(length);
  TCODFOV_dda_compute_orthogonal(
      begin.at(0), begin.at(1), end.at(0), end.at(1), length, reinterpret_cast<int*>(line.data()));
  return line;
}

/** Dummy callback for older bresenham functions. */
bool null_bresenham_callback([[maybe_unused]] int x, [[maybe_unused]] int y) { return true; }

TEST_CASE("TCODFOV_line_step_mt") {
  const std::vector<std::array<int, 2>> EXPECTED = {
      {0, 0}, {1, 0}, {2, 1}, {3, 1}, {4, 1}, {5, 1}, {6, 2}, {7, 2}, {8, 2}, {9, 2}, {10, 3}, {11, 3}};
  REQUIRE(generate_line({0, 0}, {11, 3}) == EXPECTED);

  const std::vector<std::array<int, 2>> EXPECTED2 = {
      {11, 3}, {10, 3}, {9, 2}, {8, 2}, {7, 2}, {6, 2}, {5, 1}, {4, 1}, {3, 1}, {2, 1}, {1, 0}, {0, 0}};
  REQUIRE(generate_line({11, 3}, {0, 0}) == EXPECTED2);
}

TEST_CASE("TCODFOV_dda_compute") {
  const std::vector<Point2D> EXPECTED = {
      {0, 0}, {1, 0}, {2, 1}, {3, 1}, {4, 1}, {5, 1}, {6, 2}, {7, 2}, {8, 2}, {9, 2}, {10, 3}, {11, 3}};
  REQUIRE(generate_line_dda({0, 0}, {11, 3}) == EXPECTED);

  const std::vector<Point2D> EXPECTED2 = {
      {11, 3}, {10, 3}, {9, 2}, {8, 2}, {7, 2}, {6, 2}, {5, 1}, {4, 1}, {3, 1}, {2, 1}, {1, 0}, {0, 0}};
  REQUIRE(generate_line_dda({11, 3}, {0, 0}) == EXPECTED2);
}

TEST_CASE("TCODFOV_dda_compute_orthogonal") {
  const std::vector<Point2D> EXPECTED = {
      {0, 0},
      {1, 0},
      {2, 0},
      {2, 1},
      {3, 1},
      {4, 1},
      {5, 1},
      {5, 2},
      {6, 2},
      {7, 2},
      {8, 2},
      {9, 2},
      {9, 3},
      {10, 3},
      {11, 3}};
  REQUIRE(generate_line_dda_orthogonal({0, 0}, {11, 3}) == EXPECTED);

  const std::vector<Point2D> EXPECTED2 = {
      {11, 3},
      {10, 3},
      {9, 3},
      {9, 2},
      {8, 2},
      {7, 2},
      {6, 2},
      {6, 1},
      {5, 1},
      {4, 1},
      {3, 1},
      {2, 1},
      {2, 0},
      {1, 0},
      {0, 0}};
  REQUIRE(generate_line_dda_orthogonal({11, 3}, {0, 0}) == EXPECTED2);
}

TEST_CASE("bresenham benchmarks", "[.benchmark][los]") {
  BENCHMARK("TCODFOV_line_step_mt") {
    TCODFOV_bresenham_data_t bresenham_stack_data;
    int x;
    int y;
    TCODFOV_line_init_mt(0, 0, 11, 3, &bresenham_stack_data);
    while (!TCODFOV_line_step_mt(&x, &y, &bresenham_stack_data)) {
    };
    return x;
  };
  BENCHMARK("TCODFOV_line") { return TCODFOV_line(0, 0, 11, 3, null_bresenham_callback); };
  BENCHMARK("BresenhamLine (iterate)") {
    std::array<int, 2> out{0, 0};
    for (auto&& xy : tcod::BresenhamLine({0, 0}, {11, 3})) {
      out = xy;
    }
    return out;
  };
  BENCHMARK("BresenhamLine (to vector)") {
    tcod::BresenhamLine it{{0, 0}, {11, 3}};
    return std::vector<std::array<int, 2>>{it.begin(), it.end()};
  };

  BENCHMARK("TCODFOV_dda_compute") { return generate_line_dda({0, 0}, {11, 3}); };
  BENCHMARK("TCODFOV_dda_compute_orthogonal") { return generate_line_dda_orthogonal({0, 0}, {11, 3}); };

  {
    std::vector<Point2D> line_preallocated(TCODFOV_dda_compute(0, 0, 11, 3, 0, nullptr));
    BENCHMARK("TCODFOV_dda_compute preallocated") {
      return TCODFOV_dda_compute(
          0, 0, 11, 3, std::ssize(line_preallocated), reinterpret_cast<int*>(line_preallocated.data()));
    };
  }

  {
    std::vector<Point2D> line_preallocated(TCODFOV_dda_compute_orthogonal(0, 0, 11, 3, 0, nullptr));
    BENCHMARK("TCODFOV_dda_compute_orthogonal preallocated") {
      return TCODFOV_dda_compute_orthogonal(
          0, 0, 11, 3, std::ssize(line_preallocated), reinterpret_cast<int*>(line_preallocated.data()));
    };
  }
}

TEST_CASE("BresenhamLine") {
  using Point2_Vector = std::vector<tcod::BresenhamLine::Point2>;
  const auto dest_x = GENERATE(-11, -3, 0, 3, 11);
  const auto dest_y = GENERATE(-11, -3, 0, 3, 11);
  const auto EXPECTED = generate_line({0, 0}, {dest_x, dest_y});
  SECTION("BresenhamLine compares equal to the original algorithm.") {
    const tcod::BresenhamLine bresenham_iterator{{0, 0}, {dest_x, dest_y}};
    const Point2_Vector line{bresenham_iterator.begin(), bresenham_iterator.end()};
    REQUIRE(line == EXPECTED);
  }
  SECTION("BresenhamLine endpoints can be clipped.") {
    const auto EXPECTED_CLIPPED =
        Point2_Vector{EXPECTED.begin() + 1, std::max(EXPECTED.begin() + 1, EXPECTED.end() - 1)};
    const auto bresenham_iterator = tcod::BresenhamLine({0, 0}, {dest_x, dest_y}).adjust_range(1, -1);
    const Point2_Vector line{bresenham_iterator.begin(), bresenham_iterator.end()};
    REQUIRE(line == EXPECTED_CLIPPED);
  }
}

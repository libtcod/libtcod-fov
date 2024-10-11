#pragma once
#ifndef TCODFOV_MAP_INLINE_H_
#define TCODFOV_MAP_INLINE_H_
#include "map_types.h"

/// @brief Return the width of a 2D map.
/// @param map Map union pointer, can be NULL.
/// @return The map width in tiles, or zero if `map` is NULL.
static inline int TCODFOV_map2d_get_width(const TCODFOV_Map2D* __restrict map) {
  if (!map) return 0;
  switch (map->type) {
    case TCODFOV_MAP2D_CALLBACK:
      return map->bool_callback.width;
    case TCODFOV_MAP2D_DEPRECATED:
      return map->deprecated_map.map.width;
    default:
      return 0;
  }
}
/// @brief Return the height of a 2D map.
/// @param map Map union pointer, can be NULL.
/// @return The map height in tiles, or zero if `map` is NULL.
static inline int TCODFOV_map2d_get_height(const TCODFOV_Map2D* __restrict map) {
  if (!map) return 0;
  switch (map->type) {
    case TCODFOV_MAP2D_CALLBACK:
      return map->bool_callback.height;
    case TCODFOV_MAP2D_DEPRECATED:
      return map->deprecated_map.map.height;
    default:
      return 0;
  }
}
/// @brief Return true of `x` and `y` are in bounds of `map`.
/// @param map Map union pointer, can be NULL.
/// @param x Coordinate along width.
/// @param y Coordinate along height.
/// @return True if `map` exists and `x`,`y` are within its bounds.
static inline bool TCODFOV_map2d_in_bounds(const TCODFOV_Map2D* __restrict map, int x, int y) {
  if (!map) return false;
  if (x < 0 || y < 0) return false;
  if (x >= TCODFOV_map2d_get_width(map) || y >= TCODFOV_map2d_get_height(map)) return false;
  return true;
}
/// @brief Return the boolean value of `x`, `y` on `map`.
/// @param map Map union pointer, can be NULL.
/// @param x X coordinate.
/// @param y Y coordinate.
/// @return Boolean result, false if out-of-bounds or `map` is NULL.
static inline bool TCODFOV_map2d_get_bool(const TCODFOV_Map2D* __restrict map, int x, int y) {
  if (!map) return 0;
  if (!TCODFOV_map2d_in_bounds(map, x, y)) return 0;
  switch (map->type) {
    case TCODFOV_MAP2D_CALLBACK:
      if (!map->bool_callback.get) return 0;
      return map->bool_callback.get(map->bool_callback.userdata, x, y);
    case TCODFOV_MAP2D_DEPRECATED: {
      const TCODFOV_MapCell* cell = &map->deprecated_map.map.cells[map->deprecated_map.map.width * y + x];
      switch (map->deprecated_map.select) {
        default:
        case 0:
          return cell->transparent;
        case 1:
          return cell->walkable;
        case 2:
          return cell->fov;
      }
    }
    default:
      return 0;
  }
}
/// @brief Assign the boolean `value` to `{x, y}` on `map`.
/// @param map Map union pointer, can be NULL.
/// @param x X coordinate.
/// @param y Y coordinate.
/// @param value Assigned value.
static inline void TCODFOV_map2d_set_bool(TCODFOV_Map2D* __restrict map, int x, int y, bool value) {
  if (!map) return;
  if (!TCODFOV_map2d_in_bounds(map, x, y)) return;
  switch (map->type) {
    case TCODFOV_MAP2D_CALLBACK:
      if (!map->bool_callback.set) return;
      return map->bool_callback.set(map->bool_callback.userdata, x, y, value);
    case TCODFOV_MAP2D_DEPRECATED: {
      TCODFOV_MapCell* cell = &map->deprecated_map.map.cells[map->deprecated_map.map.width * y + x];
      switch (map->deprecated_map.select) {
        default:
        case 0:
          cell->transparent = value;
          return;
        case 1:
          cell->walkable = value;
          return;
        case 2:
          cell->fov = value;
          return;
      }
    }
    default:
      return;
  }
}
#endif  // TCODFOV_MAP_INLINE_H_

#pragma once
#ifndef TCODFOV_MAP_INLINE_H_
#define TCODFOV_MAP_INLINE_H_
#include <stddef.h>
#include <stdlib.h>

#include "fov_types.h"
#include "map_types.h"

/// @brief Return minimum byte length which can hold the given number of bits.
static inline ptrdiff_t TCODFOV_round_to_byte_(ptrdiff_t bits) { return (((bits - 1) | 7) + 1) / 8; }

/// @brief Return a new bitpacked map of the given size.
/// @return The new map, or NULL if memory could not be allocated.
static inline TCODFOV_Map2D* TCODFOV_map2d_new_bitpacked(int width, int height) {
  const ptrdiff_t byte_width = TCODFOV_round_to_byte_(width);
  TCODFOV_Map2D* map = (TCODFOV_Map2D*)calloc(1, sizeof(*map) + byte_width * height);
  if (!map) return NULL;
  map->bitpacked.shape[0] = height;
  map->bitpacked.shape[1] = width;
  map->bitpacked.y_stride = byte_width;
  map->bitpacked.data = (uint8_t*)map + sizeof(*map);
  return map;
}

/// @brief Delete a map created by any TCODFOV_map2d_new function.
static inline void TCODFOV_map2d_delete(TCODFOV_Map2D* map) {
  if (map) free(map);
}

/// @brief Return the width of a 2D map.
/// @param map Map union pointer, can be NULL.
/// @return The map width in tiles, or zero if `map` is NULL.
static inline int TCODFOV_map2d_get_width(const TCODFOV_Map2D* __restrict map) {
  if (!map) return 0;
  switch (map->type) {
    case TCODFOV_MAP2D_CALLBACK:
    case TCODFOV_MAP2D_BITPACKED:
      // Multiple structs share the same shape format
      return map->bool_callback.shape[1];
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
    case TCODFOV_MAP2D_BITPACKED:
      return map->bool_callback.shape[0];
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
      const struct TCODFOV_MapCell* cell = &map->deprecated_map.map.cells[map->deprecated_map.map.width * y + x];
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
    case TCODFOV_MAP2D_BITPACKED: {
      const uint8_t active_bit = 1 << (x % 8);
      return (map->bitpacked.data[map->bitpacked.y_stride * y + (x / 8)] & active_bit) != 0;
    }
    default:
      return 0;
  }
}

/// @brief Assign the boolean `value` to `{x, y}` on `map`. Out-of-bounds writes are ignored.
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
      map->bool_callback.set(map->bool_callback.userdata, x, y, value);
      return;
    case TCODFOV_MAP2D_DEPRECATED: {
      struct TCODFOV_MapCell* cell = &map->deprecated_map.map.cells[map->deprecated_map.map.width * y + x];
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
    case TCODFOV_MAP2D_BITPACKED: {
      const uint8_t active_bit = 1 << (x % 8);
      const ptrdiff_t index = map->bitpacked.y_stride * y + (x / 8);
      map->bitpacked.data[index] = (map->bitpacked.data[index] & ~active_bit) | (value ? active_bit : 0);
      return;
    }
    default:
      return;
  }
}
#endif  // TCODFOV_MAP_INLINE_H_

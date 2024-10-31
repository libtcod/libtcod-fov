/* BSD 3-Clause License
 *
 * Copyright © 2008-2023, Jice and the libtcod contributors.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "bresenham.h"
#include "fov.h"
#include "libtcod_int.h"
#include "map_inline.h"
#include "map_types.h"
#include "utility.h"
/**
    Cast a Bresenham ray marking tiles along the line as lit.

    `radius_squared` is the max distance or zero if there is no limit.

    If `light_walls` is true then blocking walls are marked as visible.
 */
static void cast_ray(
    const TCODFOV_Map2D* __restrict transparent,
    TCODFOV_Map2D* __restrict fov,
    int x_origin,
    int y_origin,
    int x_dest,
    int y_dest,
    int radius_squared,
    bool light_walls) {
  TCODFOV_bresenham_data_t bresenham_data;
  int current_x;
  int current_y;
  TCODFOV_line_init_mt(x_origin, y_origin, x_dest, y_dest, &bresenham_data);
  while (!TCODFOV_line_step_mt(&current_x, &current_y, &bresenham_data)) {
    if (!TCODFOV_map2d_in_bounds(fov, current_x, current_y)) {
      return;  // Out of bounds.
    }
    if (radius_squared > 0) {
      const int current_radius =
          (current_x - x_origin) * (current_x - x_origin) + (current_y - y_origin) * (current_y - y_origin);
      if (current_radius > radius_squared) {
        return;  // Outside of radius.
      }
    }
    if (!TCODFOV_map2d_get_bool(transparent, current_x, current_y)) {
      if (light_walls) TCODFOV_map2d_set_bool(fov, current_x, current_y, true);
      return;  // Blocked by wall.
    }
    // Tile is transparent.
    TCODFOV_map2d_set_bool(fov, current_x, current_y, true);
  }
}
TCODFOV_Error TCODFOV_map_compute_fov_circular_raycasting(
    const TCODFOV_Map2D* __restrict transparent,
    TCODFOV_Map2D* __restrict fov,
    int pov_x,
    int pov_y,
    int max_radius,
    bool light_walls) {
  int x_min = 0;  // Field-of-view bounds.
  int y_min = 0;
  int x_max = TCODFOV_map2d_get_width(fov);
  int y_max = TCODFOV_map2d_get_height(fov);
  if (max_radius > 0) {
    x_min = MAX(x_min, pov_x - max_radius);
    y_min = MAX(y_min, pov_y - max_radius);
    x_max = MIN(x_max, pov_x + max_radius + 1);
    y_max = MIN(y_max, pov_y + max_radius + 1);
  }
  if (!TCODFOV_map2d_in_bounds(fov, pov_x, pov_y)) {
    TCODFOV_set_errorvf("Point of view {%i, %i} is out of bounds.", pov_x, pov_y);
    return TCODFOV_E_INVALID_ARGUMENT;
  }
  TCODFOV_map2d_set_bool(fov, pov_x, pov_y, true);  // Mark point-of-view as visible.

  // Cast rays along the perimeter.
  const int radius_squared = max_radius * max_radius;
  for (int x = x_min; x < x_max; ++x) {
    cast_ray(transparent, fov, pov_x, pov_y, x, y_min, radius_squared, light_walls);
  }
  for (int y = y_min + 1; y < y_max; ++y) {
    cast_ray(transparent, fov, pov_x, pov_y, x_max - 1, y, radius_squared, light_walls);
  }
  for (int x = x_max - 2; x >= x_min; --x) {
    cast_ray(transparent, fov, pov_x, pov_y, x, y_max - 1, radius_squared, light_walls);
  }
  for (int y = y_max - 2; y > y_min; --y) {
    cast_ray(transparent, fov, pov_x, pov_y, x_min, y, radius_squared, light_walls);
  }
  if (light_walls) {
    TCODFOV_map_postprocess(transparent, fov, pov_x, pov_y, max_radius);
  }
  return TCODFOV_E_OK;
}

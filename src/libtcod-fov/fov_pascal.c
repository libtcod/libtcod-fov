#include "fov_pascal.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "map_inline.h"

/// @file fov_pascal.c
/// Pascal’s triangle diffusion, based on a modifed version of this article:
/// https://towardsdatascience.com/a-quick-and-clear-look-at-grid-based-visibility-bf63769fbc78

static void pascal_scan_line(
    const TCODFOV_Map2D* __restrict transparent,  // Input transparency
    TCODFOV_Map2D* __restrict out,
    int pov_x,  // X position
    int scan_y,  // Y position
    int iteration,  // Distance from pov_y, always `abs(scan_y - pov_y)`
    const double* __restrict prev_row,  // Previous row data
    double* __restrict next_row,  // Active row to be written
    int x_begin,
    int x_end,
    int_fast8_t x_step  // Step direction: -1 or 1
) {
  for (int x = x_begin; x != x_end; x += x_step) {
    int_fast8_t casts = 0;  // Number of tiles cast from
    double visiblity = 0.0;

    // Cast diagonal
    ++casts;
    visiblity += prev_row[x - x_step];

    // Simple case will cause visiblity to cross the diagonal partition. Not sure if this is a major issue yet.
    if (pov_x - iteration <= x && x <= pov_x + iteration) {  // Cast from previous row
      ++casts;
      visiblity += prev_row[x];
    }
    if (x <= pov_x - iteration || pov_x + iteration <= x) {  // Cast from adjacent tile
      ++casts;
      visiblity += next_row[x - x_step];
    }
    visiblity *= (1.0 / (double)casts);
    TCODFOV_map2d_set_d(out, x, scan_y, visiblity);
    if (visiblity) visiblity *= TCODFOV_map2d_get_d(transparent, x, scan_y);
    next_row[x] = visiblity;
  }
}

/// @brief Compute the next row of visiblity, continue until out-of-bounds
static void pascal_scan_next_row(
    const TCODFOV_Map2D* __restrict transparent,  // Input transparency
    TCODFOV_Map2D* __restrict out,
    int pov_x,  // X position
    int scan_y,  // Y position
    int_fast8_t scan_dir,  // Y step direction, -1 or 1
    int iteration,  // Distance from pov_y, always `abs(scan_y - pov_y)`
    double* __restrict prev_row,  // Previous row light-level
    double* __restrict next_row  // This row light-level
) {
  if (scan_y < 0 || scan_y >= TCODFOV_map2d_get_height(out)) return;  // Finished, out-of-bounds

  // Compute tile at pov_x, scan_y
  TCODFOV_map2d_set_d(out, pov_x, scan_y, prev_row[pov_x]);
  next_row[pov_x] = prev_row[pov_x] * TCODFOV_map2d_get_d(transparent, pov_x, scan_y);

  // Compute tiles on sides of active row
  pascal_scan_line(transparent, out, pov_x, scan_y, iteration, prev_row, next_row, pov_x - 1, -1, -1);
  pascal_scan_line(
      transparent, out, pov_x, scan_y, iteration, prev_row, next_row, pov_x + 1, TCODFOV_map2d_get_width(out), 1);

  // Swap next_row and prev_row and continue
  pascal_scan_next_row(transparent, out, pov_x, scan_y + scan_dir, scan_dir, iteration + 1, next_row, prev_row);
}

static void pascal_scan_init(
    const TCODFOV_Map2D* __restrict transparent,  // Input transparency
    TCODFOV_Map2D* __restrict out,
    int pov_x,
    int pov_y,
    double* __restrict row  // Holds the light level this tile will cast onto others
) {
  // Source always starts at 1.0
  TCODFOV_map2d_set_d(out, pov_x, pov_y, 1.0);
  // But the data stored in row is multiplied by the maps transparency
  row[pov_x] = TCODFOV_map2d_get_d(transparent, pov_x, pov_y);
  double visiblity = row[pov_x];  // Visiblity of the active cast
  for (int x = pov_x - 1; x >= 0; --x) {
    // Output visiblity is the light level cast from the previous tile.
    // But the value stored in row is after the transparency is applied.
    TCODFOV_map2d_set_d(out, x, pov_y, visiblity);
    if (visiblity != 0) visiblity *= TCODFOV_map2d_get_d(transparent, x, pov_y);
    row[x] = visiblity;
  }
  // Repeat for the other direction
  visiblity = row[pov_x];
  for (int x = pov_x + 1; x < TCODFOV_map2d_get_width(out); ++x) {
    TCODFOV_map2d_set_d(out, x, pov_y, visiblity);
    if (visiblity != 0) visiblity *= TCODFOV_map2d_get_d(transparent, x, pov_y);
    row[x] = visiblity;
  }
}

TCODFOV_Error TCODFOV_pascal_diffusion_2d(
    const TCODFOV_Map2D* __restrict transparent, TCODFOV_Map2D* __restrict out, int pov_x, int pov_y) {
  // Holds the light-level to cast to adjacent rows
  double* __restrict row = malloc(TCODFOV_map2d_get_width(out) * 3 * sizeof(*row));

  if (!row) {
    TCODFOV_set_errorv("Out of memory.");
    return TCODFOV_E_OUT_OF_MEMORY;
  }

  double* __restrict row2 = row + TCODFOV_map2d_get_width(out);
  double* __restrict row3 = row + TCODFOV_map2d_get_width(out) * 2;

  pascal_scan_init(transparent, out, pov_x, pov_y, row);

  memcpy(row2, row, TCODFOV_map2d_get_width(out) * sizeof(*row));
  pascal_scan_next_row(transparent, out, pov_x, pov_y - 1, -1, 1, row2, row3);
  memcpy(row2, row, TCODFOV_map2d_get_width(out) * sizeof(*row));
  pascal_scan_next_row(transparent, out, pov_x, pov_y + 1, 1, 1, row2, row3);
  return TCODFOV_E_OK;
}

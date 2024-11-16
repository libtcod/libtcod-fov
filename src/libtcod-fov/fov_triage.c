#include "fov_triage.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "map_inline.h"

/// @brief Compute the reachability of tiles on this side of the row.
static void triage_scan_line(
    const TCODFOV_Map2D* __restrict transparent,  // Input transparency
    int pov_x,  // X position
    int scan_y,  // Y position
    int iteration,  // Distance from pov_y, always `abs(scan_y - pov_y)`
    const int8_t* __restrict prev_row,  // Previous row data
    int8_t* __restrict next_row,  // Active row to be written
    int x_begin,
    int x_end,
    int_fast8_t x_step  // Step direction: -1 or 1
) {
  for (int x = x_begin; x != x_end; x += x_step) {
    int_fast8_t tests = 0;  // Number of tiles checked
    int_fast8_t always_hit = 0;  // Number of checked tiles which are guaranteed visible
    int_fast8_t maybe_hit = 0;  // Number of checked tiles which are maybe visible

    // Check diagonal
    ++tests;
    if (prev_row[x - x_step] & 0b101) ++maybe_hit;
    if (prev_row[x - x_step] & 0b110) ++always_hit;

    if (pov_x - iteration <= x && x <= pov_x + iteration) {  // Check previous row
      ++tests;
      if (prev_row[x] & 0b101) ++maybe_hit;
      if (prev_row[x] & 0b110) ++always_hit;
    }
    if (x <= pov_x - iteration || pov_x + iteration <= x) {  // Check adjacent tile
      ++tests;
      if (next_row[x - x_step] & 0b101) ++maybe_hit;
      if (next_row[x - x_step] & 0b110) ++always_hit;
    }
    next_row[x] = 0;
    // If any checked tile is maybe visible, then this tile is maybe visible
    next_row[x] |= maybe_hit ? 0b1 : 0;
    // If all checked tiles are always visible then this tile is always visible
    next_row[x] |= (always_hit == tests) ? 0b10 : 0;
    next_row[x] |= (next_row[x] && TCODFOV_map2d_get_bool(transparent, x, scan_y) ? 0b100 : 0);
  }
}

/// @brief Compute the next row of visiblity, continue until out-of-bounds
static void triage_scan_next_row(
    const TCODFOV_Map2D* __restrict transparent,  // Input transparency
    TCODFOV_Map2D* __restrict out,  // Triage output
    int pov_x,  // X position
    int scan_y,  // Y position
    int_fast8_t scan_dir,  // Y step direction, -1 or 1
    int iteration,  // Distance from pov_y, always `abs(scan_y - pov_y)`
    int8_t* __restrict prev_row,  // Previous row buffer
    int8_t* __restrict next_row  // Active row buffer
) {
  if (scan_y < 0 || scan_y >= TCODFOV_map2d_get_height(out)) return;  // Finished, out-of-bounds

  // Compute tile at pov_x, scan_y
  next_row[pov_x] = prev_row[pov_x] & 0b100 ? prev_row[pov_x] : 0;
  if (next_row[pov_x] && TCODFOV_map2d_get_bool(transparent, pov_x, scan_y)) next_row[pov_x] &= 0b11;

  // Compute tiles on sides of active row
  triage_scan_line(transparent, pov_x, scan_y, iteration, prev_row, next_row, pov_x - 1, -1, -1);
  triage_scan_line(
      transparent, pov_x, scan_y, iteration, prev_row, next_row, pov_x + 1, TCODFOV_map2d_get_width(out), 1);

  // Output triage data
  for (int x = 0; x < TCODFOV_map2d_get_width(out); ++x) {
    TCODFOV_map2d_set_int(out, x, scan_y, next_row[x] & 0b11);
  }

  // Swap next_row and prev_row and continue
  triage_scan_next_row(transparent, out, pov_x, scan_y + scan_dir, scan_dir, iteration + 1, next_row, prev_row);
}

/// @brief Compute initial triage data for the middle row.
static void triage_scan_init(
    const TCODFOV_Map2D* __restrict transparent,  // Input transparency
    TCODFOV_Map2D* __restrict out,  // Triage output
    int pov_x,
    int pov_y,
    int8_t* __restrict row) {
  row[pov_x] = TCODFOV_map2d_get_bool(transparent, pov_x, pov_y) ? 0b111 : 0b011;
  for (int x = pov_x - 1; x >= 0; --x) {
    row[x] = 0;
    if (row[x + 1] & 0b100) row[x] = TCODFOV_map2d_get_bool(transparent, x, pov_y) ? 0b111 : 0b011;
  }
  for (int x = pov_x + 1; x < TCODFOV_map2d_get_width(out); ++x) {
    row[x] = 0;
    if (row[x - 1] & 0b100) row[x] = TCODFOV_map2d_get_bool(transparent, x, pov_y) ? 0b111 : 0b011;
  }
  for (int x = 0; x < TCODFOV_map2d_get_width(out); ++x) {
    TCODFOV_map2d_set_int(out, x, pov_y, row[x] & 0b11);
  }
}

TCODFOV_Error TCODFOV_triage_2d(
    const TCODFOV_Map2D* __restrict transparent, TCODFOV_Map2D* __restrict out, int pov_x, int pov_y) {
  // Triage data on row format: 0bXYZ
  // X = transparent: caches input transparency if applicable
  // Y = always visible: all tiles to this tile are always visible
  // Z = maybe visible: at least one tile to this tile is maybe visible
  int8_t* __restrict row = malloc(TCODFOV_map2d_get_width(out) * 3 * sizeof(*row));
  if (!row) {
    TCODFOV_set_errorv("Out of memory.");
    return TCODFOV_E_OUT_OF_MEMORY;
  }
  int8_t* __restrict row2 = row + TCODFOV_map2d_get_width(out);
  int8_t* __restrict row3 = row + TCODFOV_map2d_get_width(out) * 2;

  triage_scan_init(transparent, out, pov_x, pov_y, row);

  memcpy(row2, row, TCODFOV_map2d_get_width(out) * sizeof(*row));
  triage_scan_next_row(transparent, out, pov_x, pov_y - 1, -1, 1, row2, row3);
  memcpy(row2, row, TCODFOV_map2d_get_width(out) * sizeof(*row));
  triage_scan_next_row(transparent, out, pov_x, pov_y + 1, 1, 1, row2, row3);
  return TCODFOV_E_OK;
}

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
#pragma once
#ifndef TCODLIB_INT_H_
#define TCODLIB_INT_H_
#include <stdbool.h>

#include "config.h"
#include "error.h"
#include "fov.h"
#include "fov_types.h"
#include "map_types.h"

/* tcodlib internal stuff */
#ifdef __cplusplus
extern "C" {
#endif

/* fov internal stuff */
TCODFOV_PUBLIC TCODFOV_Error TCODFOV_map_compute_fov_circular_raycasting(
    const TCODFOV_Map2D* __restrict transparent,
    TCODFOV_Map2D* __restrict fov,
    int pov_x,
    int pov_y,
    int max_radius,
    bool light_walls);
TCODFOV_PUBLIC TCODFOV_Error TCODFOV_map_compute_fov_diamond_raycasting(
    const TCODFOV_Map2D* __restrict transparent,
    TCODFOV_Map2D* __restrict fov,
    int pov_x,
    int pov_y,
    int max_radius,
    bool light_walls);
TCODFOV_PUBLIC TCODFOV_Error TCODFOV_map_compute_fov_recursive_shadowcasting(
    const TCODFOV_Map2D* __restrict transparent,
    TCODFOV_Map2D* __restrict fov,
    int pov_x,
    int pov_y,
    int max_radius,
    bool light_walls);
TCODFOV_PUBLIC TCODFOV_Error TCODFOV_map_compute_fov_permissive2(
    const TCODFOV_Map2D* __restrict transparent,
    TCODFOV_Map2D* __restrict fov,
    int pov_x,
    int pov_y,
    int max_radius,
    bool light_walls,
    int permissiveness);
TCODFOV_PUBLIC TCODFOV_Error TCODFOV_map_compute_fov_restrictive_shadowcasting(
    const TCODFOV_Map2D* __restrict transparent,
    TCODFOV_Map2D* __restrict fov,
    int pov_x,
    int pov_y,
    int max_radius,
    bool light_walls);
TCODFOV_PUBLIC TCODFOV_Error TCODFOV_map_compute_fov_symmetric_shadowcast(
    const TCODFOV_Map2D* __restrict transparent,
    TCODFOV_Map2D* __restrict fov,
    int pov_x,
    int pov_y,
    int max_radius,
    bool light_walls);
TCODFOV_PUBLIC TCODFOV_Error TCODFOV_map_postprocess(
    const TCODFOV_Map2D* __restrict transparent, TCODFOV_Map2D* __restrict fov, int pov_x, int pov_y, int radius);
/**
    Return true if `x` and `y` are in the boundaries of `map`.

    Returns false if `map` is NULL.
 */
static inline bool TCODFOV_map_in_bounds(const struct TCODFOV_Map* map, int x, int y) {
  return map && 0 <= x && x < map->width && 0 <= y && y < map->height;
}

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // TCODLIB_INT_H_

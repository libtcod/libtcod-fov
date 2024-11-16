
#pragma once
#ifndef TCODFOV_FOV_PASCAL_H_
#define TCODFOV_FOV_PASCAL_H_

#include <stdbool.h>

#include "config.h"
#include "error.h"
#include "map_types.h"

#ifdef __cplusplus
extern "C" {
#endif

TCODFOV_PUBLIC TCODFOV_Error TCODFOV_pascal_diffusion_2d(
    const TCODFOV_Map2D* __restrict transparent, TCODFOV_Map2D* __restrict out, int pov_x, int pov_y);

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // TCODFOV_FOV_PASCAL_H_

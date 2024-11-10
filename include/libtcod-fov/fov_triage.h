
#pragma once
#ifndef TCODFOV_FOV_TRIAGE_H_
#define TCODFOV_FOV_TRIAGE_H_

#include <stdbool.h>

#include "config.h"
#include "error.h"
#include "map_types.h"

TCODFOV_PUBLIC TCODFOV_Error
TCODFOV_triage_2d(const TCODFOV_Map2D* __restrict transparent, TCODFOV_Map2D* __restrict out, int pov_x, int pov_y);

#endif  // TCODFOV_FOV_TRIAGE_H_

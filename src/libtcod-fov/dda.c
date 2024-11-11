#include "dda.h"

#include <math.h>

#include "utility.h"

ptrdiff_t TCODFOV_dda_compute(
    double begin_x, double begin_y, double end_x, double end_y, ptrdiff_t out_n, int* __restrict out_xy) {
  const ptrdiff_t dx = (ptrdiff_t)(end_x - begin_x);
  const ptrdiff_t dy = (ptrdiff_t)(end_y - begin_y);
  const ptrdiff_t N = MAX(ABS(dx), ABS(dy));
  const double divN = (N == 0) ? 0.0 : 1.0 / N;
  const double x_step = dx * divN;
  const double y_step = dy * divN;
  double x = begin_x;
  double y = begin_y;
  if (out_xy && out_n > 0) {
    for (ptrdiff_t step = 0; step < out_n && out_xy; ++step, x += x_step, y += y_step) {
      *(out_xy++) = (int)floor(x + 0.5);
      *(out_xy++) = (int)floor(y + 0.5);
    }
  }
  return N + 1;
}

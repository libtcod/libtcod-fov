
/// @file dda.h
/// @brief Digital Differential Analyzer, for tile-based line rendering.
///
/// Based off of Red Blob Games DDA optimizations:
/// https://www.redblobgames.com/grids/line-drawing/#optimization
#include <stddef.h>

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif
/// @brief Compute a line using digital differential analyzer and output to an array.
///
/// First call this function with an `out_xy` of NULL to get the indicies of the line.
/// Then use the returned value to allocate a buffer of size `sizeof(int) * 2 * n`.
/// Call this function again using that buffer as the output.
///
/// @code{.cpp}
///     ptrdiff_t length = TCODFOV_dda_compute(10, 10, 25, 30, 0, NULL);
///     int* line_xy = malloc(2 * length * sizeof(*line_xy));
///     TCODFOV_dda_compute(10, 10, 25, 30, length, line_xy);
/// @endcode
///
/// Output includes the beginning coordinates.
/// Using the returned size will include the ending coordinates.
/// A smaller buffer will have a truncated output, and a larger buffer will output coordinates past the end.
/// @param begin_x Starting X coordinate.
/// @param begin_y Starting Y coordinate.
/// @param end_x Target X coordinate.
/// @param end_y Target Y coordinate.
/// @param out_n // Number of indexes to write to `out_xy`.
/// @param out_xy // Output array of contigious XY coordinates, if NULL then no data will be written.
///     If not NULL then MUST be size `sizeof(int) * 2 * out_n`.
/// @return The count of indexes needed to reach the end coordinate.
TCODFOV_PUBLIC ptrdiff_t TCODFOV_dda_compute(
    double begin_x, double begin_y, double end_x, double end_y, ptrdiff_t out_n, int* __restrict out_xy);

/// @brief Compute an orthogonal line using digital differential analyzer and output to an array.
///
/// Output includes the beginning coordinates.
/// Using the returned size will include the ending coordinates.
/// A smaller buffer will have a truncated output, and a larger buffer will output coordinates past the end.
/// @param begin_x Starting X coordinate.
/// @param begin_y Starting Y coordinate.
/// @param end_x Target X coordinate.
/// @param end_y Target Y coordinate.
/// @param out_n // Number of indexes to write to `out_xy`.
/// @param out_xy // Output array of contigious XY coordinates, if NULL then no data will be written.
///     If not NULL then MUST be size `sizeof(int) * 2 * out_n`.
/// @return The count of indexes needed to reach the end coordinate.
TCODFOV_PUBLIC ptrdiff_t TCODFOV_dda_compute_orthogonal(
    double begin_x, double begin_y, double end_x, double end_y, ptrdiff_t out_n, int* __restrict out_xy);
#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#pragma once
#ifndef TCODFOV_MAP_TYPES_H_
#define TCODFOV_MAP_TYPES_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "fov_types.h"

/// @brief 2D map types
typedef enum TCODFOV_Map2DType {
  TCODFOV_MAP2D_UNDEFINED = 0,
  TCODFOV_MAP2D_CALLBACK = 1,
  TCODFOV_MAP2D_DEPRECATED = 2,
  TCODFOV_MAP2D_BITPACKED = 3,
  TCODFOV_MAP2D_CONTIGIOUS = 4,
} TCODFOV_Map2DType;

typedef enum TCODFOV_DataType {
  TCODFOV_DATATYPE_BOOL = 0,
  TCODFOV_DATATYPE_UINT8 = 1,
  TCODFOV_DATATYPE_FLOAT = 2,
  TCODFOV_DATATYPE_DOUBLE = 3,
} TCODFOV_DataType;

/// @brief Callbacks to get/set on 2D grids.
struct TCODFOV_Map2DCallback {
  TCODFOV_Map2DType type;  // Must be TCODFOV_MAP2D_CALLBACK
  int shape[2];  // {height, width}
  void* userdata;
  bool (*get)(void* userdata, int x, int y);  // Get callback
  void (*set)(void* userdata, int x, int y, bool v);  // Set callback
};

/// @brief Comptability layer with TCODFOV_Map.
struct TCODFOV_Map2DDeprecated {
  TCODFOV_Map2DType type;  // Must be TCODFOV_MAP2D_DEPRECATED
  int select;  // 0 = transparent, 1 = walkable, 2 = fov
  TCODFOV_Map map;
};

/// @brief Bitpacked 2D grid.
struct TCODFOV_Map2DBitpacked {
  TCODFOV_Map2DType type;  // Must be TCODFOV_MAP2D_BITPACKED
  int shape[2];  // {height, width}
  uint8_t* __restrict data;  // Boolean data packed into bytes
  ptrdiff_t y_stride;  // Array stride along the y-axis
};

/// @brief Contigious 2D grid.
struct TCODFOV_Map2DContigious {
  TCODFOV_Map2DType type;  // Must be TCODFOV_MAP2D_CONTIGIOUS
  int shape[2];  // {height, width}
  unsigned char* __restrict data;  // Boolean data packed into bytes
  TCODFOV_DataType item_type;
};

/// @brief Union type for 2D maps.
typedef union TCODFOV_Map2D {
  TCODFOV_Map2DType type;
  struct TCODFOV_Map2DCallback bool_callback;
  struct TCODFOV_Map2DDeprecated deprecated_map;
  struct TCODFOV_Map2DBitpacked bitpacked;
  struct TCODFOV_Map2DContigious contigious;
} TCODFOV_Map2D;
#endif  // TCODFOV_MAP_TYPES_H_

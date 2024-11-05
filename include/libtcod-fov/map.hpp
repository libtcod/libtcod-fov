#include <memory>

#include "map_inline.h"
#include "map_types.h"

namespace tcod::fov {

struct MapDeleter {
  void operator()(TCODFOV_Map2D* map) const { TCODFOV_map2d_delete(map); }
};

typedef std::unique_ptr<TCODFOV_Map2D, MapDeleter> Map2DPtr;

}  // namespace tcod::fov

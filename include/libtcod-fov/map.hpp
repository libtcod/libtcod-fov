#include <memory>
#include <stdexcept>
#include <string>

#include "map_inline.h"
#include "map_types.h"

namespace tcod::fov {

/// @brief Deleter for TCODFOV map structs.
struct MapDeleter {
  void operator()(TCODFOV_Map2D* map) const { TCODFOV_map2d_delete(map); }
};
/// @brief Unique pointer for `TCODFOV_Map2D`.
typedef std::unique_ptr<TCODFOV_Map2D, MapDeleter> Map2DPtr;

/// @brief A 2D bitpacked boolean array.
class Bitpacked2D {
 public:
  /// @brief Iterator over a bitpacked boolean array. Acts as both a reference and pointer.
  class Bitpacked2DIter {
   public:
    Bitpacked2DIter() = default;
    Bitpacked2DIter(Bitpacked2D* map, std::array<int, 2> ij) noexcept : map_{map}, ij_{std::move(ij)} {}

    /// @brief Assign boolean to this index.
    /// @throws std::out_of_range If iterator is out-of-bounds.
    Bitpacked2DIter& operator=(bool value) {
      map_->set_bool(ij_, value);
      return *this;
    }

    /// @brief This class should act as a bool when possible.
    /// @throws std::out_of_range If iterator is out-of-bounds.
    operator bool() const { return map_->get_bool(ij_); }

    /// @brief This object is both an iterator and a reference, so return itself.
    Bitpacked2DIter& operator*() noexcept { return *this; }

    /// @brief Increment along row-major order.
    Bitpacked2DIter& operator++() noexcept {
      ++ij_[0];
      while (ij_[0] > map_->map_.bitpacked.shape[0]) {
        ++ij_[1];
        ij_[0] -= map_->map_.bitpacked.shape[0];
      }
      return *this;
    }

    bool operator==(const Bitpacked2DIter& other) const noexcept { return map_ == other.map_ && ij_ == other.ij_; }
    bool operator!=(const Bitpacked2DIter& other) const noexcept { return !(*this == other); }

   private:
    Bitpacked2D* map_{};  // Pointer to this iterators map.
    std::array<int, 2> ij_{};  // Index on the map.
  };

  /// @brief Default initialize an array with no shape.
  Bitpacked2D() : Bitpacked2D({0, 0}) {}
  /// @brief Create a new array with the given shape and inital value.
  /// @param shape `{height, width}` shape of the new array.
  /// @param init Inital value of the booleans in this array.
  explicit Bitpacked2D(const std::array<int, 2>& shape, bool init = false)
      : array_(shape[0] * (TCODFOV_round_to_byte_(shape[1])), (init ? 0xff : 0)),
        map_{
            .bitpacked{
                .type = TCODFOV_MAP2D_BITPACKED,
                .shape = {shape[0], shape[1]},
                .data = array_.data(),
                .y_stride = TCODFOV_round_to_byte_(shape[1]),
            },
        } {}

  Bitpacked2D(const Bitpacked2D&) = delete;
  Bitpacked2D& operator=(const Bitpacked2D&) = delete;
  Bitpacked2D(Bitpacked2D&&) = default;
  Bitpacked2D& operator=(Bitpacked2D&&) = default;

  auto begin() noexcept { return Bitpacked2DIter{this, {0, 0}}; }
  auto end() noexcept { return Bitpacked2DIter{this, {map_.bitpacked.shape[0], 0}}; }

  /// @brief Return the shape of the array as `{height, width}`.
  auto get_shape() const noexcept -> std::array<int, 2> { return {map_.bitpacked.shape[0], map_.bitpacked.shape[1]}; }

  /// @brief Return the width of this array.
  auto get_width() const noexcept -> int { return map_.bitpacked.shape[1]; }
  /// @brief Return the height of this array.
  auto get_height() const noexcept -> int { return map_.bitpacked.shape[0]; }

  /// @brief Return the boolean at `ij`.
  /// @throws std::out_of_range If `ij` is out-of-bounds.
  auto get_bool(const std::array<int, 2>& ij) const -> bool {
    check_bounds_(ij);
    const uint8_t active_bit = 1 << (ij[1] % 8);
    return (array_.at(map_.bitpacked.y_stride * ij[0] + (ij[1] / 8)) & active_bit) != 0;
  }

  /// @brief Set the boolean at `ij` to `value`.
  /// @throws std::out_of_range If `ij` is out-of-bounds.
  auto set_bool(const std::array<int, 2>& ij, bool value) -> void {
    check_bounds_(ij);
    const uint8_t active_bit = 1 << (ij[1] % 8);
    auto& byte_ref = array_.at(map_.bitpacked.y_stride * ij[0] + (ij[1] / 8));
    byte_ref = (byte_ref & ~active_bit) | (value ? active_bit : 0);
  }

  /// @brief Return true if `ij` is within the bounds of this array.
  auto in_bounds(const std::array<int, 2>& ij) const noexcept -> bool {
    return 0 <= ij[0] && 0 <= ij[1] && ij[0] < map_.bitpacked.shape[0] && ij[1] < map_.bitpacked.shape[1];
  }

  /// @brief Return the `TCODFOV_Map2D*` pointer for this object.
  auto get_ptr() noexcept { return static_cast<TCODFOV_Map2D*>(*this); }
  /// @brief Return the `const TCODFOV_Map2D*` pointer for this object.
  auto get_ptr() const noexcept { return static_cast<const TCODFOV_Map2D*>(*this); }

  /// @brief Conversion to `TCODFOV_Map2D*` so that this may be passed to C FOV functions.
  explicit operator TCODFOV_Map2D*() noexcept { return &map_; }
  /// @brief Conversion to `const TCODFOV_Map2D*` so that this may be passed to C FOV functions.
  explicit operator const TCODFOV_Map2D*() const noexcept { return &map_; }

 private:
  /// @brief Throw `std::out_of_range` if `ij` is out-of-bounds.
  void check_bounds_(const std::array<int, 2>& ij) const {
    if (in_bounds(ij)) return;
    throw std::out_of_range(
        std::string("{") + std::to_string(ij[0]) + ", " + std::to_string(ij[1]) +
        "} not in bounds of array of shape {" + std::to_string(map_.bitpacked.shape[0]) + ", " +
        std::to_string(map_.bitpacked.shape[1]) + "}");
  }
  std::vector<uint8_t> array_{};  // Bitpacked map data.
  TCODFOV_Map2D map_{};  // Map union in memory, includes a pointer to `array_`.
};

}  // namespace tcod::fov

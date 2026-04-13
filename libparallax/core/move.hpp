// SPDX-License-Identifier: MIT
#ifndef LIBPARALLAX_CORE_MOVE_HPP_
#define LIBPARALLAX_CORE_MOVE_HPP_

#include <cstdint>

#include <array>

#include <fmt/format.h>

namespace parallax {

enum class square : std::uint8_t {
  a1, b1, c1, d1, e1, f1, g1, h1,
  a2, b2, c2, d2, e2, f2, g2, h2,
  a3, b3, c3, d3, e3, f3, g3, h3,
  a4, b4, c4, d4, e4, f4, g4, h4,
  a5, b5, c5, d5, e5, f5, g5, h5,
  a6, b6, c6, d6, e6, f6, g6, h6,
  a7, b7, c7, d7, e7, f7, g7, h7,
  a8, b8, c8, d8, e8, f8, g8, h8,
  none
}; // enum class square

enum class move_flag : std::uint8_t {
  quiet = 0,
  double_push = 1,
  castle_king = 2,
  castle_queen = 3,
  capture = 4,
  en_passant = 5,
  promo_knight = 8,
  promo_bishop = 9,
  promo_rook = 10,
  promo_queen = 11,
  promo_capture_knight  = 12,
  promo_capture_bishop = 13,
  promo_capture_rook = 14,
  promo_capture_queen = 15,
}; // enum class move_flag

class move {

public:

  constexpr move() noexcept = default;

  constexpr move(square from, square to, move_flag flag) noexcept;

  constexpr auto from() const noexcept -> square;

  constexpr auto to() const noexcept -> square;

  constexpr auto flag() const noexcept -> move_flag;

  constexpr auto is_capture() const noexcept -> bool;

  constexpr auto is_promotion() const noexcept -> bool;

  constexpr auto raw() const noexcept -> std::uint16_t;

  friend constexpr auto operator==(const move& lhs, const move& rhs) noexcept -> bool = default;

private:

  std::uint16_t _data{0};

};

} // namespace parallax

template<>
struct fmt::formatter<parallax::move> : fmt::formatter<std::string_view> {

  using base = fmt::formatter<std::string_view>;

  template<typename FormatContext>
  auto format(const parallax::move& move, FormatContext& context) const noexcept -> decltype(context.out());

}; // struct std::formatter

#endif // LIBPARALLAX_CORE_MOVE_HPP_
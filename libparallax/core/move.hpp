// SPDX-License-Identifier: MIT
#ifndef LIBPARALLAX_CORE_MOVE_HPP_
#define LIBPARALLAX_CORE_MOVE_HPP_

#include <cstdint>

#include <array>
#include <string_view>

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
  quiet = 0u,
  double_push = 1,
  castle_king = 2,
  castle_queen = 3,
  capture = 4,
  en_passant = 5,
  promo_knight = 8,
  promo_bishop = 9,
  promo_rook = 10,
  promo_queen = 11,
  promo_capture_knight = 12,
  promo_capture_bishop = 13,
  promo_capture_rook = 14,
  promo_capture_queen = 15,
}; // enum class move_flag

constexpr auto make_square(std::uint32_t file, std::uint32_t rank) noexcept -> square {
  return static_cast<square>(rank * 8 + file);
}

constexpr auto file_of(const square square) noexcept -> std::int32_t {
  return static_cast<std::int32_t>(square) & 7;
}

constexpr auto rank_of(const square square) noexcept -> std::int32_t {
  return static_cast<std::int32_t>(square) >> 3;
}

class move {

public:

  constexpr move() noexcept = default;

  constexpr move(const square from, const square to, const move_flag flag) noexcept
  : _data{static_cast<std::uint16_t>((static_cast<std::uint16_t>(from) & 0x3F) | ((static_cast<std::uint16_t>(to) & 0x3F) << 6) | ((static_cast<std::uint16_t>(flag) & 0x0F) << 12))} { }

  constexpr auto from() const noexcept -> square {
    return static_cast<square>(_data & 0x3F);
  }

  constexpr auto to() const noexcept -> square {
    return static_cast<square>((_data >> 6) & 0x3F);
  }

  constexpr auto flag() const noexcept -> move_flag {
    return static_cast<move_flag>((_data >> 12) & 0x0F);
  }

  constexpr auto is_capture() const noexcept -> bool {
    return ((_data >> 12) & 0x4) != 0;
  }

  constexpr auto is_promotion() const noexcept -> bool {
    return ((_data >> 12) & 0x8) != 0;
  }

  constexpr auto raw() const noexcept -> std::uint16_t {
    return _data;
  }

  friend constexpr auto operator==(move lhs, move rhs) noexcept -> bool = default;

private:

  std::uint16_t _data{0};

}; // class move

} // namespace parallax

template<>
struct fmt::formatter<parallax::move> : fmt::formatter<std::string_view> {

  using base = fmt::formatter<std::string_view>;

  template<typename FormatContext>
  auto format(parallax::move move, FormatContext& context) const -> decltype(context.out()) {
    auto buffer = std::array<char, 6>{};
    auto from = static_cast<std::uint8_t>(move.from());
    auto to = static_cast<std::uint8_t>(move.to());

    buffer[0] = 'a' + (from & 7);
    buffer[1] = '1' + (from >> 3);
    buffer[2] = 'a' + (to & 7);
    buffer[3] = '1' + (to >> 3);

    auto length = 4uz;

    if (move.is_promotion()) {
      constexpr auto promotion = std::array{'n', 'b', 'r', 'q'};

      buffer[4] = promotion[static_cast<std::uint8_t>(move.flag()) & 0x3];
      length = 5;
    }

    return base::format(std::string_view{buffer.data(), length}, context);
  }

}; // struct fmt::formatter<parallax::move>

#endif // LIBPARALLAX_CORE_MOVE_HPP_

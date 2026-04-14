// SPDX-License-Identifier: MIT
#ifndef LIBPARALLAX_CORE_ATTACKS_HPP_
#define LIBPARALLAX_CORE_ATTACKS_HPP_

#include <array>

#include <libparallax/core/bitboard.hpp>
#include <libparallax/core/move.hpp>
#include <libparallax/core/position.hpp>

namespace parallax {

namespace detail {

constexpr auto compute_knight_attacks(const square origin) noexcept -> bitboard {
  const auto source = square_to_bit(origin);

  auto attacks = bitboard{0};

  attacks |= (source & ~file_h)            << 17; // north north east
  attacks |= (source & ~file_a)            << 15; // north north west
  attacks |= (source & ~(file_g | file_h)) << 10; // north east east
  attacks |= (source & ~(file_a | file_b)) << 6;  // north west west
  attacks |= (source & ~file_a)            >> 17; // south south west
  attacks |= (source & ~file_h)            >> 15; // south south east
  attacks |= (source & ~(file_a | file_b)) >> 10; // south west west
  attacks |= (source & ~(file_g | file_h)) >> 6;  // south east east

  return attacks;
}

constexpr auto compute_king_attacks(const square origin) noexcept -> bitboard {
  const auto source = square_to_bit(origin);

  auto attacks = bitboard{0};

  attacks |= shift_north(source);
  attacks |= shift_south(source);
  attacks |= shift_east(source);
  attacks |= shift_west(source);
  attacks |= shift_north_east(source);
  attacks |= shift_north_west(source);
  attacks |= shift_south_east(source);
  attacks |= shift_south_west(source);

  return attacks;
}

constexpr auto compute_pawn_attacks(const color piece_color, const square origin) noexcept -> bitboard {
  const auto source = square_to_bit(origin);

  if (piece_color == color::white) {
    return shift_north_east(source) | shift_north_west(source);
  }

  return shift_south_east(source) | shift_south_west(source);
}

constexpr auto build_knight_table() noexcept -> std::array<bitboard, 64> {
  auto table = std::array<bitboard, 64>{};

  for (auto index = 0u; index < 64u; ++index) {
    table[index] = compute_knight_attacks(static_cast<square>(index));
  }

  return table;
}

constexpr auto build_king_table() noexcept -> std::array<bitboard, 64> {
  auto table = std::array<bitboard, 64>{};

  for (auto index = 0u; index < 64u; ++index) {
    table[index] = compute_king_attacks(static_cast<square>(index));
  }

  return table;
}

constexpr auto build_pawn_table(const color piece_color) noexcept -> std::array<bitboard, 64> {
  auto table = std::array<bitboard, 64>{};

  for (auto index = 0u; index < 64u; ++index) {
    table[index] = compute_pawn_attacks(piece_color, static_cast<square>(index));
  }

  return table;
}

inline constexpr auto bishop_directions = std::array{9, 7, -7, -9};
inline constexpr auto rook_directions = std::array{8, 1, -1, -8};

constexpr auto ray_attacks(const square origin, const std::int32_t direction, const bitboard occupancy) noexcept -> bitboard {
  auto attacks = bitboard{0};

  auto current_index = static_cast<std::int32_t>(origin);
  auto previous_file = file_of(static_cast<square>(current_index));
  auto previous_rank = rank_of(static_cast<square>(current_index));

  while (true) {
    const auto next_index = current_index + direction;

    if (next_index < 0 || next_index >= 64) {
      break;
    }

    const auto next_square = static_cast<square>(next_index);
    const auto next_file = file_of(next_square);
    const auto next_rank = rank_of(next_square);

    const auto file_delta = next_file - previous_file;
    const auto rank_delta = next_rank - previous_rank;

    if (file_delta < -1 || file_delta > 1 || rank_delta < -1 || rank_delta > 1) {
      break;
    }

    const auto next_bit = square_to_bit(next_square);
    attacks |= next_bit;

    if (occupancy & next_bit) {
      break;
    }

    current_index = next_index;
    previous_file = next_file;
    previous_rank = next_rank;
  }

  return attacks;
}

} // namespace detail

inline constexpr auto knight_attacks_table = detail::build_knight_table();
inline constexpr auto king_attacks_table = detail::build_king_table();
inline constexpr auto white_pawn_attacks_table = detail::build_pawn_table(color::white);
inline constexpr auto black_pawn_attacks_table = detail::build_pawn_table(color::black);

constexpr auto knight_attacks(const square origin) noexcept -> bitboard {
  return knight_attacks_table[static_cast<std::size_t>(origin)];
}

constexpr auto king_attacks(const square origin) noexcept -> bitboard {
  return king_attacks_table[static_cast<std::size_t>(origin)];
}

constexpr auto pawn_attacks(const color piece_color, const square origin) noexcept -> bitboard {
  return piece_color == color::white ? white_pawn_attacks_table[static_cast<std::size_t>(origin)] : black_pawn_attacks_table[static_cast<std::size_t>(origin)];
}

constexpr auto bishop_attacks(const square origin, const bitboard occupancy) noexcept -> bitboard {
  auto attacks = bitboard{0};

  for (const auto direction : detail::bishop_directions) {
    attacks |= detail::ray_attacks(origin, direction, occupancy);
  }

  return attacks;
}

constexpr auto rook_attacks(const square origin, const bitboard occupancy) noexcept -> bitboard {
  auto attacks = bitboard{0};

  for (const auto direction : detail::rook_directions) {
    attacks |= detail::ray_attacks(origin, direction, occupancy);
  }

  return attacks;
}

constexpr auto queen_attacks(const square origin, const bitboard occupancy) noexcept -> bitboard {
  return bishop_attacks(origin, occupancy) | rook_attacks(origin, occupancy);
}

} // namespace parallax

#endif // LIBPARALLAX_CORE_ATTACKS_HPP_

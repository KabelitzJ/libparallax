// SPDX-License-Identifier: MIT
#ifndef LIBPARALLAX_CORE_ZOBRIST_HPP_
#define LIBPARALLAX_CORE_ZOBRIST_HPP_

#include <array>
#include <cstdint>

#include <libparallax/core/move.hpp>

namespace parallax {

enum class color : std::uint8_t;
enum class piece : std::uint8_t;

namespace zobrist {

namespace detail {

constexpr auto splitmix64(const std::uint64_t state) noexcept -> std::uint64_t {
  auto result = state + 0x9E3779B97F4A7C15ULL;

  result = (result ^ (result >> 30)) * 0xBF58476D1CE4E5B9ULL;
  result = (result ^ (result >> 27)) * 0x94D049BB133111EBULL;

  return result ^ (result >> 31);
}

constexpr auto build_piece_table() noexcept -> std::array<std::array<std::array<std::uint64_t, 64>, 6>, 2> {
  auto table = std::array<std::array<std::array<std::uint64_t, 64>, 6>, 2>{};
  auto state = std::uint64_t{0x1234567890ABCDEFULL};

  for (auto color_index = 0uz; color_index < 2; ++color_index) {
    for (auto piece_index = 0uz; piece_index < 6; ++piece_index) {
      for (auto square_index = 0uz; square_index < 64; ++square_index) {
        state = splitmix64(state);
        table[color_index][piece_index][square_index] = state;
      }
    }
  }

  return table;
}

constexpr auto build_castling_table() noexcept -> std::array<std::uint64_t, 16> {
  auto table = std::array<std::uint64_t, 16>{};
  auto state = std::uint64_t{0xFEDCBA0987654321ULL};

  for (auto index = 0uz; index < 16; ++index) {
    state = splitmix64(state);
    table[index] = state;
  }

  return table;
}

constexpr auto build_en_passant_table() noexcept -> std::array<std::uint64_t, 8> {
  auto table = std::array<std::uint64_t, 8>{};
  auto state = std::uint64_t{0xABCDEF0123456789ULL};

  for (auto index = 0uz; index < 8; ++index) {
    state = splitmix64(state);
    table[index] = state;
  }

  return table;
}

constexpr auto build_side_to_move_key() noexcept -> std::uint64_t {
  return splitmix64(0xCAFEBABEDEADBEEFULL);
}

} // namespace detail

inline constexpr auto piece_square_keys = detail::build_piece_table();
inline constexpr auto castling_keys = detail::build_castling_table();
inline constexpr auto en_passant_file_keys = detail::build_en_passant_table();
inline constexpr auto side_to_move_key = detail::build_side_to_move_key();

constexpr auto piece_square_key(const color piece_color, const piece piece_type, const square target_square) noexcept -> std::uint64_t {
  return piece_square_keys[static_cast<std::size_t>(piece_color)][static_cast<std::size_t>(piece_type)][static_cast<std::size_t>(target_square)];
}

} // namespace zobrist

} // namespace parallax

#endif // LIBPARALLAX_CORE_ZOBRIST_HPP_

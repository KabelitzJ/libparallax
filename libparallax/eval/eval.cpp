// SPDX-License-Identifier: MIT
#include <libparallax/eval/eval.hpp>

#include <array>

#include <libparallax/core/bitboard.hpp>

namespace parallax {

constexpr auto piece_values = std::array<std::uint32_t, 6>{
  100,   // pawn
  320,   // knight
  330,   // bishop
  500,   // rook
  900,   // queen
  0,     // king (not counted; can't be captured)
};

auto evaluate(const position& current_position) noexcept -> std::int32_t {
  auto white_score = std::uint32_t{0};
  auto black_score = std::uint32_t{0};

  for (auto piece_index = 0u; piece_index < 6u; ++piece_index) {
    const auto piece_type = static_cast<piece>(piece_index);
    const auto value = piece_values[piece_index];

    white_score += value * popcount(current_position.pieces(color::white, piece_type));
    black_score += value * popcount(current_position.pieces(color::black, piece_type));
  }

  const auto raw_score = static_cast<std::int32_t>(white_score) - static_cast<std::int32_t>(black_score);

  return current_position.side_to_move() == color::white ? raw_score : -raw_score;
}

} // namespace parallax
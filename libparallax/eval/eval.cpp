// SPDX-License-Identifier: MIT
#include <libparallax/eval/eval.hpp>

#include <array>

#include <libparallax/core/bitboard.hpp>

namespace parallax {

constexpr auto piece_values = std::array<int, 6>{
  100,   // pawn
  320,   // knight
  330,   // bishop
  500,   // rook
  900,   // queen
  0,     // king (not counted; can't be captured)
};

auto evaluate(const position& current_position) noexcept -> int {
  auto white_score = 0;
  auto black_score = 0;

  for (auto piece_index = 0uz; piece_index < 6; ++piece_index) {
    const auto piece_type = static_cast<piece>(piece_index);
    const auto value = piece_values[piece_index];

    white_score += value * popcount(current_position.pieces(color::white, piece_type));
    black_score += value * popcount(current_position.pieces(color::black, piece_type));
  }

  const auto raw_score = white_score - black_score;

  return current_position.side_to_move() == color::white ? raw_score : -raw_score;
}

} // namespace parallax
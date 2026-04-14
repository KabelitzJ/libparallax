// SPDX-License-Identifier: MIT
#include <libparallax/eval/eval.hpp>

#include <array>

#include <libparallax/core/bitboard.hpp>

namespace parallax {

constexpr auto piece_values = std::array<std::int32_t, 6>{
  100,   // pawn
  320,   // knight
  330,   // bishop
  500,   // rook
  900,   // queen
  0,     // king
};

// PSTs are written from white's perspective, rank 8 at top, rank 1 at bottom.
// Index 0 = a1, index 63 = h8. This matches your square enum.
// For black, we mirror the rank: black_square = white_square ^ 56.

constexpr auto pawn_pst = std::array<std::int32_t, 64>{
   0,  0,  0,  0,  0,  0,  0,  0,
   5, 10, 10,-20,-20, 10, 10,  5,
   5, -5,-10,  0,  0,-10, -5,  5,
   0,  0,  0, 20, 20,  0,  0,  0,
   5,  5, 10, 25, 25, 10,  5,  5,
  10, 10, 20, 30, 30, 20, 10, 10,
  50, 50, 50, 50, 50, 50, 50, 50,
   0,  0,  0,  0,  0,  0,  0,  0,
};

constexpr auto knight_pst = std::array<std::int32_t, 64>{
  -50,-40,-30,-30,-30,-30,-40,-50,
  -40,-20,  0,  5,  5,  0,-20,-40,
  -30,  5, 10, 15, 15, 10,  5,-30,
  -30,  0, 15, 20, 20, 15,  0,-30,
  -30,  5, 15, 20, 20, 15,  5,-30,
  -30,  0, 10, 15, 15, 10,  0,-30,
  -40,-20,  0,  0,  0,  0,-20,-40,
  -50,-40,-30,-30,-30,-30,-40,-50,
};

constexpr auto bishop_pst = std::array<std::int32_t, 64>{
  -20,-10,-10,-10,-10,-10,-10,-20,
  -10,  5,  0,  0,  0,  0,  5,-10,
  -10, 10, 10, 10, 10, 10, 10,-10,
  -10,  0, 10, 10, 10, 10,  0,-10,
  -10,  5,  5, 10, 10,  5,  5,-10,
  -10,  0,  5, 10, 10,  5,  0,-10,
  -10,  0,  0,  0,  0,  0,  0,-10,
  -20,-10,-10,-10,-10,-10,-10,-20,
};

constexpr auto rook_pst = std::array<std::int32_t, 64>{
   0,  0,  0,  5,  5,  0,  0,  0,
  -5,  0,  0,  0,  0,  0,  0, -5,
  -5,  0,  0,  0,  0,  0,  0, -5,
  -5,  0,  0,  0,  0,  0,  0, -5,
  -5,  0,  0,  0,  0,  0,  0, -5,
  -5,  0,  0,  0,  0,  0,  0, -5,
   5, 10, 10, 10, 10, 10, 10,  5,
   0,  0,  0,  0,  0,  0,  0,  0,
};

constexpr auto queen_pst = std::array<std::int32_t, 64>{
  -20,-10,-10, -5, -5,-10,-10,-20,
  -10,  0,  5,  0,  0,  0,  0,-10,
  -10,  5,  5,  5,  5,  5,  0,-10,
    0,  0,  5,  5,  5,  5,  0, -5,
   -5,  0,  5,  5,  5,  5,  0, -5,
  -10,  0,  5,  5,  5,  5,  0,-10,
  -10,  0,  0,  0,  0,  0,  0,-10,
  -20,-10,-10, -5, -5,-10,-10,-20,
};

constexpr auto king_pst = std::array<std::int32_t, 64>{
   20, 30, 10,  0,  0, 10, 30, 20,
   20, 20,  0,  0,  0,  0, 20, 20,
  -10,-20,-20,-20,-20,-20,-20,-10,
  -20,-30,-30,-40,-40,-30,-30,-20,
  -30,-40,-40,-50,-50,-40,-40,-30,
  -30,-40,-40,-50,-50,-40,-40,-30,
  -30,-40,-40,-50,-50,-40,-40,-30,
  -30,-40,-40,-50,-50,-40,-40,-30,
};

constexpr auto piece_square_tables = std::array{
  pawn_pst,
  knight_pst,
  bishop_pst,
  rook_pst,
  queen_pst,
  king_pst,
};

auto score_side(const position& current_position, const color piece_color) noexcept -> std::int32_t {
  auto score = std::int32_t{0};

  for (auto piece_index = 0uz; piece_index < 6; ++piece_index) {
    const auto piece_type = static_cast<piece>(piece_index);
    const auto value = piece_values[piece_index];
    const auto& table = piece_square_tables[piece_index];

    auto pieces = current_position.pieces(piece_color, piece_type);

    while (pieces != 0) {
      const auto target_square = pop_lsb(pieces);
      const auto table_index = piece_color == color::white ? static_cast<std::size_t>(target_square) : static_cast<std::size_t>(target_square) ^ 56uz;

      score += value + table[table_index];
    }
  }

  return score;
}

auto evaluate(const position& current_position) noexcept -> std::int32_t {
  const auto white_score = score_side(current_position, color::white);
  const auto black_score = score_side(current_position, color::black);

  const auto raw_score = white_score - black_score;

  return current_position.side_to_move() == color::white ? raw_score : -raw_score;
}

} // namespace parallax

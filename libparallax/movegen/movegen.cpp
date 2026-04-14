// SPDX-License-Identifier: MIT
#include <libparallax/movegen/movegen.hpp>

#include <libparallax/core/attacks.hpp>
#include <libparallax/core/bitboard.hpp>

namespace parallax {

namespace {

constexpr auto white_kingside_path = square_to_bit(square::f1) | square_to_bit(square::g1);
constexpr auto white_queenside_path = square_to_bit(square::b1) | square_to_bit(square::c1) | square_to_bit(square::d1);
constexpr auto black_kingside_path = square_to_bit(square::f8) | square_to_bit(square::g8);
constexpr auto black_queenside_path = square_to_bit(square::b8) | square_to_bit(square::c8) | square_to_bit(square::d8);

auto add_pawn_promotions(std::vector<move>& moves, const square from_square, const square to_square, const bool is_capture) -> void {
  if (is_capture) {
    moves.emplace_back(from_square, to_square, move_flag::promo_capture_queen);
    moves.emplace_back(from_square, to_square, move_flag::promo_capture_rook);
    moves.emplace_back(from_square, to_square, move_flag::promo_capture_bishop);
    moves.emplace_back(from_square, to_square, move_flag::promo_capture_knight);
  } else {
    moves.emplace_back(from_square, to_square, move_flag::promo_queen);
    moves.emplace_back(from_square, to_square, move_flag::promo_rook);
    moves.emplace_back(from_square, to_square, move_flag::promo_bishop);
    moves.emplace_back(from_square, to_square, move_flag::promo_knight);
  }
}

auto generate_pawn_moves(const position& current_position, std::vector<move>& moves) -> void {
  const auto mover_color = current_position.side_to_move();
  const auto pawns = current_position.pieces(mover_color, piece::pawn);
  const auto enemy_occupancy = current_position.occupancy(mover_color == color::white ? color::black : color::white);
  const auto all_occupancy = current_position.occupancy();
  const auto empty_squares = ~all_occupancy;

  const auto is_white = mover_color == color::white;
  const auto promotion_rank = is_white ? rank_8 : rank_1;
  const auto double_push_rank = is_white ? rank_4 : rank_5;

  const auto single_push_targets = (is_white ? shift_north(pawns) : shift_south(pawns)) & empty_squares;
  const auto double_push_targets = (is_white ? shift_north(single_push_targets) : shift_south(single_push_targets)) & empty_squares & double_push_rank;

  const auto push_offset = is_white ? -8 : 8;
  const auto double_push_offset = is_white ? -16 : 16;

  for (const auto target_square : squares_of(single_push_targets & ~promotion_rank)) {
    const auto from_square = static_cast<square>(static_cast<int>(target_square) + push_offset);
    moves.emplace_back(from_square, target_square, move_flag::quiet);
  }

  for (const auto target_square : squares_of(single_push_targets & promotion_rank)) {
    const auto from_square = static_cast<square>(static_cast<int>(target_square) + push_offset);
    add_pawn_promotions(moves, from_square, target_square, false);
  }

  for (const auto target_square : squares_of(double_push_targets)) {
    const auto from_square = static_cast<square>(static_cast<int>(target_square) + double_push_offset);
    moves.emplace_back(from_square, target_square, move_flag::double_push);
  }

  for (const auto from_square : squares_of(pawns)) {
    const auto attack_set = pawn_attacks(mover_color, from_square);
    const auto capture_targets = attack_set & enemy_occupancy;

    for (const auto target_square : squares_of(capture_targets & ~promotion_rank)) {
      moves.emplace_back(from_square, target_square, move_flag::capture);
    }

    for (const auto target_square : squares_of(capture_targets & promotion_rank)) {
      add_pawn_promotions(moves, from_square, target_square, true);
    }
  }
}

auto generate_en_passant_moves(const position& current_position, std::vector<move>& moves) -> void {
  const auto ep_square = current_position.en_passant_square();

  if (ep_square == square::none) {
    return;
  }

  const auto mover_color = current_position.side_to_move();
  const auto pawns = current_position.pieces(mover_color, piece::pawn);
  const auto enemy_color = mover_color == color::white ? color::black : color::white;

  const auto attackers = pawn_attacks(enemy_color, ep_square) & pawns;

  for (const auto from_square : squares_of(attackers)) {
    moves.emplace_back(from_square, ep_square, move_flag::en_passant);
  }
}

auto generate_castling_moves(const position& current_position, std::vector<move>& moves) -> void {
  const auto mover_color = current_position.side_to_move();
  const auto rights = current_position.castling_rights();
  const auto all_occupancy = current_position.occupancy();
  const auto enemy_color = mover_color == color::white ? color::black : color::white;

  if (current_position.is_square_attacked(mover_color == color::white ? square::e1 : square::e8, enemy_color)) {
    return;
  }

  if (mover_color == color::white) {
    if ((rights & castling::white_king) && (all_occupancy & white_kingside_path) == 0) {
      if (!current_position.is_square_attacked(square::f1, enemy_color) && !current_position.is_square_attacked(square::g1, enemy_color)) {
        moves.emplace_back(square::e1, square::g1, move_flag::castle_king);
      }
    }

    if ((rights & castling::white_queen) && (all_occupancy & white_queenside_path) == 0) {
      if (!current_position.is_square_attacked(square::d1, enemy_color) && !current_position.is_square_attacked(square::c1, enemy_color)) {
        moves.emplace_back(square::e1, square::c1, move_flag::castle_queen);
      }
    }
  } else {
    if ((rights & castling::black_king) && (all_occupancy & black_kingside_path) == 0) {
      if (!current_position.is_square_attacked(square::f8, enemy_color) && !current_position.is_square_attacked(square::g8, enemy_color)) {
        moves.emplace_back(square::e8, square::g8, move_flag::castle_king);
      }
    }

    if ((rights & castling::black_queen) && (all_occupancy & black_queenside_path) == 0) {
      if (!current_position.is_square_attacked(square::d8, enemy_color) && !current_position.is_square_attacked(square::c8, enemy_color)) {
        moves.emplace_back(square::e8, square::c8, move_flag::castle_queen);
      }
    }
  }
}

auto generate_knight_moves(const position& current_position, std::vector<move>& moves) -> void {
  const auto mover_color = current_position.side_to_move();
  const auto knights = current_position.pieces(mover_color, piece::knight);
  const auto own_occupancy = current_position.occupancy(mover_color);
  const auto enemy_occupancy = current_position.occupancy(mover_color == color::white ? color::black : color::white);

  for (const auto from_square : squares_of(knights)) {
    const auto attack_set = knight_attacks(from_square) & ~own_occupancy;

    for (const auto target_square : squares_of(attack_set & enemy_occupancy)) {
      moves.emplace_back(from_square, target_square, move_flag::capture);
    }

    for (const auto target_square : squares_of(attack_set & ~enemy_occupancy)) {
      moves.emplace_back(from_square, target_square, move_flag::quiet);
    }
  }
}

auto generate_sliding_moves(const position& current_position, std::vector<move>& moves, const piece piece_type) -> void {
  const auto mover_color = current_position.side_to_move();
  const auto sliders = current_position.pieces(mover_color, piece_type);
  const auto own_occupancy = current_position.occupancy(mover_color);
  const auto enemy_occupancy = current_position.occupancy(mover_color == color::white ? color::black : color::white);
  const auto all_occupancy = current_position.occupancy();

  for (const auto from_square : squares_of(sliders)) {
    auto attack_set = bitboard{0};

    if (piece_type == piece::bishop) {
      attack_set = bishop_attacks(from_square, all_occupancy);
    } else if (piece_type == piece::rook) {
      attack_set = rook_attacks(from_square, all_occupancy);
    } else {
      attack_set = queen_attacks(from_square, all_occupancy);
    }

    attack_set &= ~own_occupancy;

    for (const auto target_square : squares_of(attack_set & enemy_occupancy)) {
      moves.emplace_back(from_square, target_square, move_flag::capture);
    }

    for (const auto target_square : squares_of(attack_set & ~enemy_occupancy)) {
      moves.emplace_back(from_square, target_square, move_flag::quiet);
    }
  }
}

auto generate_king_moves(const position& current_position, std::vector<move>& moves) -> void {
  const auto mover_color = current_position.side_to_move();
  const auto king_board = current_position.pieces(mover_color, piece::king);

  if (king_board == 0) {
    return;
  }

  const auto from_square = lsb(king_board);
  const auto own_occupancy = current_position.occupancy(mover_color);
  const auto enemy_occupancy = current_position.occupancy(mover_color == color::white ? color::black : color::white);
  const auto attack_set = king_attacks(from_square) & ~own_occupancy;

  for (const auto target_square : squares_of(attack_set & enemy_occupancy)) {
    moves.emplace_back(from_square, target_square, move_flag::capture);
  }

  for (const auto target_square : squares_of(attack_set & ~enemy_occupancy)) {
    moves.emplace_back(from_square, target_square, move_flag::quiet);
  }
}

} // namespace

auto generate_pseudo_legal_moves(const position& current_position) -> std::vector<move> {
  auto moves = std::vector<move>{};
  moves.reserve(64);

  generate_pawn_moves(current_position, moves);
  generate_en_passant_moves(current_position, moves);
  generate_knight_moves(current_position, moves);
  generate_sliding_moves(current_position, moves, piece::bishop);
  generate_sliding_moves(current_position, moves, piece::rook);
  generate_sliding_moves(current_position, moves, piece::queen);
  generate_king_moves(current_position, moves);
  generate_castling_moves(current_position, moves);

  return moves;
}

auto generate_legal_moves(const position& current_position) -> std::vector<move> {
  auto pseudo_legal = generate_pseudo_legal_moves(current_position);
  auto legal_moves = std::vector<move>{};
  legal_moves.reserve(pseudo_legal.size());

  auto working_position = current_position;
  const auto mover_color = current_position.side_to_move();
  const auto enemy_color = mover_color == color::white ? color::black : color::white;

  for (const auto candidate : pseudo_legal) {
    working_position.make_move(candidate);

    const auto king_board = working_position.pieces(mover_color, piece::king);

    if (king_board != 0) {
      const auto king_square = lsb(king_board);

      if (!working_position.is_square_attacked(king_square, enemy_color)) {
        legal_moves.push_back(candidate);
      }
    }

    working_position.unmake_move();
  }

  return legal_moves;
}

} // namespace parallax

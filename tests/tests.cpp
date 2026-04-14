// SPDX-License-Identifier: MIT
#include <vector>

#include <gtest/gtest.h>

#include <libparallax/core/core.hpp>
#include <libparallax/movegen/movegen.hpp>
#include <libparallax/perft/perft.hpp>
#include <libparallax/uci/uci.hpp>
#include <libparallax/eval/eval.hpp>
#include <libparallax/search/search.hpp>

using namespace parallax;

TEST(fen, startpos_roundtrip) {
  constexpr auto startpos = std::string_view{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"};

  const auto parsed = position::from_fen(startpos);

  ASSERT_TRUE(parsed.has_value()) << parsed.error();
  EXPECT_EQ(parsed->to_fen(), startpos);
}

TEST(fen, kiwipete_roundtrip) {
  constexpr auto fen = std::string_view{"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"};

  const auto parsed = position::from_fen(fen);

  ASSERT_TRUE(parsed.has_value()) << parsed.error();
  EXPECT_EQ(parsed->to_fen(), fen);
}

TEST(fen, position_3_roundtrip) {
  constexpr auto fen = std::string_view{"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"};

  const auto parsed = position::from_fen(fen);

  ASSERT_TRUE(parsed.has_value()) << parsed.error();
  EXPECT_EQ(parsed->to_fen(), fen);
}

TEST(fen, position_4_roundtrip) {
  constexpr auto fen = std::string_view{"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2pP/R2Q1RK1 w kq - 0 1"};

  const auto parsed = position::from_fen(fen);

  ASSERT_TRUE(parsed.has_value()) << parsed.error();
  EXPECT_EQ(parsed->to_fen(), fen);
}

TEST(fen, ep_square_parsed) {
  constexpr auto fen = std::string_view{"rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2"};

  const auto parsed = position::from_fen(fen);

  ASSERT_TRUE(parsed.has_value()) << parsed.error();
  EXPECT_EQ(parsed->to_fen(), fen);
}

TEST(fen, rejects_too_few_fields) {
  EXPECT_FALSE(position::from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -").has_value());
}

TEST(fen, rejects_bad_piece_char) {
  EXPECT_FALSE(position::from_fen("xnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1").has_value());
}

TEST(fen, rejects_short_rank) {
  EXPECT_FALSE(position::from_fen("rnbqkbn/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1").has_value());
}

TEST(bitboard, popcount_basic) {
  EXPECT_EQ(popcount(0), 0);
  EXPECT_EQ(popcount(0xFFULL), 8);
  EXPECT_EQ(popcount(~bitboard{0}), 64);
}

TEST(bitboard, lsb_and_pop) {
  auto board = square_to_bit(square::e4) | square_to_bit(square::a1) | square_to_bit(square::h8);

  EXPECT_EQ(lsb(board), square::a1);

  const auto first = pop_lsb(board);
  EXPECT_EQ(first, square::a1);
  EXPECT_EQ(popcount(board), 2);

  const auto second = pop_lsb(board);
  EXPECT_EQ(second, square::e4);

  const auto third = pop_lsb(board);
  EXPECT_EQ(third, square::h8);
  EXPECT_EQ(board, bitboard{0});
}

TEST(bitboard, shifts_no_wrap) {
  EXPECT_EQ(shift_east(file_h), bitboard{0});
  EXPECT_EQ(shift_west(file_a), bitboard{0});
  EXPECT_EQ(shift_north(rank_8), bitboard{0});
  EXPECT_EQ(shift_south(rank_1), bitboard{0});
}

TEST(bitboard, shift_north_east_corner) {
  EXPECT_EQ(shift_north_east(square_to_bit(square::e4)), square_to_bit(square::f5));
  EXPECT_EQ(shift_north_east(square_to_bit(square::h4)), bitboard{0});
}

TEST(bitboard, iteration) {
  const auto board = square_to_bit(square::a1) | square_to_bit(square::d4) | square_to_bit(square::h8);

  auto collected = std::vector<square>{};

  for (const auto target_square : squares_of(board)) {
    collected.push_back(target_square);
  }

  ASSERT_EQ(collected.size(), 3uz);
  EXPECT_EQ(collected[0], square::a1);
  EXPECT_EQ(collected[1], square::d4);
  EXPECT_EQ(collected[2], square::h8);
}

TEST(attacks, knight_d4_has_eight_targets) {
  const auto attacks = knight_attacks(square::d4);

  EXPECT_EQ(popcount(attacks), 8);

  const auto expected
    = square_to_bit(square::b3) | square_to_bit(square::b5)
    | square_to_bit(square::c2) | square_to_bit(square::c6)
    | square_to_bit(square::e2) | square_to_bit(square::e6)
    | square_to_bit(square::f3) | square_to_bit(square::f5);

  EXPECT_EQ(attacks, expected);
}

TEST(attacks, knight_a1_has_two_targets) {
  const auto attacks = knight_attacks(square::a1);

  EXPECT_EQ(popcount(attacks), 2);
  EXPECT_EQ(attacks, square_to_bit(square::b3) | square_to_bit(square::c2));
}

TEST(attacks, knight_h8_has_two_targets) {
  const auto attacks = knight_attacks(square::h8);

  EXPECT_EQ(popcount(attacks), 2);
  EXPECT_EQ(attacks, square_to_bit(square::f7) | square_to_bit(square::g6));
}

TEST(attacks, king_e4_has_eight_targets) {
  const auto attacks = king_attacks(square::e4);

  EXPECT_EQ(popcount(attacks), 8);
}

TEST(attacks, king_a1_has_three_targets) {
  const auto attacks = king_attacks(square::a1);

  EXPECT_EQ(popcount(attacks), 3);
  EXPECT_EQ(attacks, square_to_bit(square::a2) | square_to_bit(square::b1) | square_to_bit(square::b2));
}

TEST(attacks, white_pawn_e4_attacks_d5_f5) {
  const auto attacks = pawn_attacks(color::white, square::e4);

  EXPECT_EQ(attacks, square_to_bit(square::d5) | square_to_bit(square::f5));
}

TEST(attacks, black_pawn_e5_attacks_d4_f4) {
  const auto attacks = pawn_attacks(color::black, square::e5);

  EXPECT_EQ(attacks, square_to_bit(square::d4) | square_to_bit(square::f4));
}

TEST(attacks, white_pawn_a4_attacks_b5_only) {
  const auto attacks = pawn_attacks(color::white, square::a4);

  EXPECT_EQ(attacks, square_to_bit(square::b5));
}

TEST(attacks, knight_total_moves_is_336) {
  auto total = 0u;

  for (auto index = 0u; index < 64u; ++index) {
    total += popcount(knight_attacks(static_cast<square>(index)));
  }

  EXPECT_EQ(total, 336);
}

TEST(attacks, rook_on_empty_board_d4) {
  const auto attacks = rook_attacks(square::d4, 0);

  EXPECT_EQ(popcount(attacks), 14);
}

TEST(attacks, bishop_on_empty_board_d4) {
  const auto attacks = bishop_attacks(square::d4, 0);

  EXPECT_EQ(popcount(attacks), 13);
}

TEST(attacks, queen_on_empty_board_d4) {
  const auto attacks = queen_attacks(square::d4, 0);

  EXPECT_EQ(popcount(attacks), 27);
}

TEST(attacks, rook_blocked_by_friendly) {
  const auto occupancy = square_to_bit(square::d4) | square_to_bit(square::d6) | square_to_bit(square::f4);
  const auto attacks = rook_attacks(square::d4, occupancy);

  const auto expected
    = square_to_bit(square::d5) | square_to_bit(square::d6)
    | square_to_bit(square::e4) | square_to_bit(square::f4)
    | square_to_bit(square::c4) | square_to_bit(square::b4) | square_to_bit(square::a4)
    | square_to_bit(square::d3) | square_to_bit(square::d2) | square_to_bit(square::d1);

  EXPECT_EQ(attacks, expected);
}

TEST(attacks, bishop_blocked_by_blocker) {
  const auto occupancy = square_to_bit(square::f6);
  const auto attacks = bishop_attacks(square::d4, occupancy);

  EXPECT_TRUE(attacks & square_to_bit(square::e5));
  EXPECT_TRUE(attacks & square_to_bit(square::f6));
  EXPECT_FALSE(attacks & square_to_bit(square::g7));
  EXPECT_FALSE(attacks & square_to_bit(square::h8));
}

TEST(attacks, rook_corner_a1_empty_board) {
  const auto attacks = rook_attacks(square::a1, 0);

  EXPECT_EQ(popcount(attacks), 14);
}

TEST(attacks, bishop_corner_a1_empty_board) {
  const auto attacks = bishop_attacks(square::a1, 0);

  EXPECT_EQ(popcount(attacks), 7);
}

TEST(attacks, rook_no_wrap_h4_east) {
  const auto attacks = rook_attacks(square::h4, 0);

  EXPECT_FALSE(attacks & square_to_bit(square::a5));
  EXPECT_TRUE(attacks & square_to_bit(square::a4));
  EXPECT_EQ(popcount(attacks), 14);
}

TEST(make_move, quiet_pawn_push_roundtrip) {
  auto pos = position::from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1").value();
  const auto original_fen = pos.to_fen();

  pos.make_move(move{square::e2, square::e3, move_flag::quiet});
  EXPECT_EQ(pos.piece_at(square::e2), piece::none);
  EXPECT_EQ(pos.piece_at(square::e3), piece::pawn);
  EXPECT_EQ(pos.side_to_move(), color::black);

  pos.unmake_move();
  EXPECT_EQ(pos.to_fen(), original_fen);
}

TEST(make_move, double_push_sets_ep_square) {
  auto pos = position::from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1").value();

  pos.make_move(move{square::e2, square::e4, move_flag::double_push});
  EXPECT_EQ(pos.to_fen(), "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");

  pos.unmake_move();
  EXPECT_EQ(pos.to_fen(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

TEST(make_move, capture_roundtrip) {
  auto pos = position::from_fen("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2").value();
  const auto original_fen = pos.to_fen();

  pos.make_move(move{square::e4, square::d5, move_flag::capture});
  EXPECT_EQ(pos.piece_at(square::d5), piece::pawn);
  EXPECT_EQ(pos.color_at(square::d5), color::white);

  pos.unmake_move();
  EXPECT_EQ(pos.to_fen(), original_fen);
}

TEST(make_move, en_passant_roundtrip) {
  auto pos = position::from_fen("rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3").value();
  const auto original_fen = pos.to_fen();

  pos.make_move(move{square::e5, square::f6, move_flag::en_passant});
  EXPECT_EQ(pos.piece_at(square::f6), piece::pawn);
  EXPECT_EQ(pos.piece_at(square::f5), piece::none);

  pos.unmake_move();
  EXPECT_EQ(pos.to_fen(), original_fen);
}

TEST(make_move, kingside_castle_roundtrip) {
  auto pos = position::from_fen("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1").value();
  const auto original_fen = pos.to_fen();

  pos.make_move(move{square::e1, square::g1, move_flag::castle_king});
  EXPECT_EQ(pos.piece_at(square::g1), piece::king);
  EXPECT_EQ(pos.piece_at(square::f1), piece::rook);
  EXPECT_EQ(pos.piece_at(square::e1), piece::none);
  EXPECT_EQ(pos.piece_at(square::h1), piece::none);

  pos.unmake_move();
  EXPECT_EQ(pos.to_fen(), original_fen);
}

TEST(make_move, queenside_castle_roundtrip) {
  auto pos = position::from_fen("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1").value();
  const auto original_fen = pos.to_fen();

  pos.make_move(move{square::e1, square::c1, move_flag::castle_queen});
  EXPECT_EQ(pos.piece_at(square::c1), piece::king);
  EXPECT_EQ(pos.piece_at(square::d1), piece::rook);

  pos.unmake_move();
  EXPECT_EQ(pos.to_fen(), original_fen);
}

TEST(make_move, promotion_to_queen_roundtrip) {
  auto pos = position::from_fen("8/P7/8/8/8/8/8/4k2K w - - 0 1").value();
  const auto original_fen = pos.to_fen();

  pos.make_move(move{square::a7, square::a8, move_flag::promo_queen});
  EXPECT_EQ(pos.piece_at(square::a8), piece::queen);
  EXPECT_EQ(pos.piece_at(square::a7), piece::none);

  pos.unmake_move();
  EXPECT_EQ(pos.to_fen(), original_fen);
}

TEST(make_move, capture_promotion_roundtrip) {
  auto pos = position::from_fen("1n6/P7/8/8/8/8/8/4k2K w - - 0 1").value();
  const auto original_fen = pos.to_fen();

  pos.make_move(move{square::a7, square::b8, move_flag::promo_capture_queen});
  EXPECT_EQ(pos.piece_at(square::b8), piece::queen);
  EXPECT_EQ(pos.color_at(square::b8), color::white);

  pos.unmake_move();
  EXPECT_EQ(pos.to_fen(), original_fen);
}

TEST(make_move, rook_move_clears_castling_right) {
  auto pos = position::from_fen("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1").value();

  pos.make_move(move{square::h1, square::h2, move_flag::quiet});

  // KQkq -> Qkq (white kingside lost)
  const auto fen_after = pos.to_fen();
  EXPECT_NE(fen_after.find("Qkq "), std::string::npos);
  EXPECT_EQ(fen_after.find("KQ"), std::string::npos);
}

TEST(in_check, white_king_attacked_by_rook) {
  auto pos = position::from_fen("4k3/8/8/8/8/8/8/r3K3 w - - 0 1").value();
  EXPECT_TRUE(pos.in_check());
}

TEST(in_check, white_king_safe) {
  auto pos = position::from_fen("4k3/8/8/8/8/8/8/4K3 w - - 0 1").value();
  EXPECT_FALSE(pos.in_check());
}

TEST(movegen, startpos_has_twenty_moves) {
  auto pos = position::from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1").value();

  const auto moves = generate_legal_moves(pos);

  EXPECT_EQ(moves.size(), 20uz);
}

TEST(movegen, startpos_black_has_twenty_moves) {
  auto pos = position::from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1").value();

  const auto moves = generate_legal_moves(pos);

  EXPECT_EQ(moves.size(), 20uz);
}

TEST(movegen, kiwipete_has_fortyeight_moves) {
  auto pos = position::from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1").value();

  const auto moves = generate_legal_moves(pos);

  EXPECT_EQ(moves.size(), 48uz);
}

TEST(movegen, only_king_can_move_when_in_check) {
  auto pos = position::from_fen("4k3/8/8/8/8/8/4r3/4K3 w - - 0 1").value();

  const auto moves = generate_legal_moves(pos);

  for (const auto candidate : moves) {
    EXPECT_EQ(pos.piece_at(candidate.from()), piece::king);
  }
}

TEST(movegen, pinned_piece_cannot_move_off_pin_ray) {
  auto pos = position::from_fen("4k3/8/8/8/8/4r3/4N3/4K3 w - - 0 1").value();

  const auto moves = generate_legal_moves(pos);

  for (const auto candidate : moves) {
    EXPECT_NE(pos.piece_at(candidate.from()), piece::knight);
  }
}

TEST(movegen, castling_blocked_by_attack_through_path) {
  auto pos = position::from_fen("4k3/8/8/8/8/8/5r2/4K2R w K - 0 1").value();

  const auto moves = generate_legal_moves(pos);

  for (const auto candidate : moves) {
    EXPECT_NE(candidate.flag(), move_flag::castle_king);
  }
}

TEST(movegen, castling_allowed_when_path_clear) {
  auto pos = position::from_fen("4k3/8/8/8/8/8/8/4K2R w K - 0 1").value();

  const auto moves = generate_legal_moves(pos);

  auto found_castle = false;

  for (const auto candidate : moves) {
    if (candidate.flag() == move_flag::castle_king) {
      found_castle = true;
      break;
    }
  }

  EXPECT_TRUE(found_castle);
}

TEST(perft, startpos_depth_1) {
  auto pos = position::from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1").value();

  EXPECT_EQ(perft(pos, 1), 20ULL);
}

TEST(perft, startpos_depth_2) {
  auto pos = position::from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1").value();

  EXPECT_EQ(perft(pos, 2), 400ULL);
}

TEST(perft, startpos_depth_3) {
  auto pos = position::from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1").value();

  EXPECT_EQ(perft(pos, 3), 8902ULL);
}

TEST(perft, startpos_depth_4) {
  auto pos = position::from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1").value();

  EXPECT_EQ(perft(pos, 4), 197281ULL);
}

TEST(perft, kiwipete_depth_1) {
  auto pos = position::from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1").value();

  EXPECT_EQ(perft(pos, 1), 48ULL);
}

TEST(perft, kiwipete_depth_2) {
  auto pos = position::from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1").value();

  EXPECT_EQ(perft(pos, 2), 2039ULL);
}

TEST(perft, kiwipete_depth_3) {
  auto pos = position::from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1").value();

  EXPECT_EQ(perft(pos, 3), 97862ULL);
}

TEST(perft, position_3_depth_4) {
  auto pos = position::from_fen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1").value();

  EXPECT_EQ(perft(pos, 4), 43238ULL);
}

TEST(perft, position_4_depth_3) {
  auto pos = position::from_fen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2pP/R2Q1RK1 w kq - 0 1").value();

  EXPECT_EQ(perft(pos, 3), 9346ULL);
}

TEST(perft, position_5_depth_3) {
  auto pos = position::from_fen("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8").value();

  EXPECT_EQ(perft(pos, 3), 62379ULL);
}

TEST(perft, divide_kiwipete_depth_2) {
  auto pos = position::from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1").value();

  perft_divide(pos, 2);
}

TEST(perft, divide_position_4_depth_1) {
  auto pos = position::from_fen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2pP/R2Q1RK1 w kq - 0 1").value();
  perft_divide(pos, 1);
}

TEST(perft, divide_position_4_depth_2) {
  auto pos = position::from_fen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2pP/R2Q1RK1 w kq - 0 1").value();
  perft_divide(pos, 2);
}

TEST(perft, startpos_depth_5) {
  auto pos = position::from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1").value();

  EXPECT_EQ(perft(pos, 5), 4865609ULL);
}

TEST(perft, kiwipete_depth_4) {
  auto pos = position::from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1").value();

  EXPECT_EQ(perft(pos, 4), 4085603ULL);
}

TEST(perft, position_3_depth_5) {
  auto pos = position::from_fen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1").value();

  EXPECT_EQ(perft(pos, 5), 674624ULL);
}

TEST(perft, position_4_depth_4) {
  auto pos = position::from_fen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2pP/R2Q1RK1 w kq - 0 1").value();

  EXPECT_EQ(perft(pos, 4), 438345ULL);
}

TEST(perft, position_5_depth_4) {
  auto pos = position::from_fen("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8").value();

  EXPECT_EQ(perft(pos, 4), 2103487ULL);
}

TEST(movegen, debug_pin) {
  auto pos = position::from_fen("4k2r/8/8/8/8/8/4N3/4K3 w - - 0 1").value();

  fmt::println("knight at e2: {}", pos.piece_at(square::e2) == piece::knight);
  fmt::println("rook at e8: {}", pos.piece_at(square::e8) == piece::rook);
  fmt::println("king at e1: {}", pos.piece_at(square::e1) == piece::king);

  fmt::println("e1 attacked by black (before any move): {}", pos.is_square_attacked(square::e1, color::black));

  auto copy = pos;
  copy.make_move(move{square::e2, square::c3, move_flag::quiet});

  fmt::println("after Nc3:");
  fmt::println("  knight at c3: {}", copy.piece_at(square::c3) == piece::knight);
  fmt::println("  e2 empty: {}", copy.piece_at(square::e2) == piece::none);
  fmt::println("  e1 attacked by black: {}", copy.is_square_attacked(square::e1, color::black));
  fmt::println("  side to move: {}", copy.side_to_move() == color::white ? "white" : "black");
}

TEST(uci, identifies_on_uci_command) {
  auto input = std::istringstream{"uci\nquit\n"};
  auto output = std::ostringstream{};

  run_uci_loop(input, output);

  const auto result = output.str();

  EXPECT_NE(result.find("id name"), std::string::npos);
  EXPECT_NE(result.find("id author"), std::string::npos);
  EXPECT_NE(result.find("uciok"), std::string::npos);
}

TEST(uci, responds_readyok) {
  auto input = std::istringstream{"isready\nquit\n"};
  auto output = std::ostringstream{};

  run_uci_loop(input, output);

  EXPECT_NE(output.str().find("readyok"), std::string::npos);
}

TEST(uci, go_from_startpos_emits_bestmove) {
  auto input = std::istringstream{"position startpos\ngo\nquit\n"};
  auto output = std::ostringstream{};

  run_uci_loop(input, output);

  const auto result = output.str();

  EXPECT_NE(result.find("bestmove"), std::string::npos);
}

TEST(uci, position_with_moves_applied) {
  auto input = std::istringstream{"position startpos moves e2e4 e7e5\ngo\nquit\n"};
  auto output = std::ostringstream{};

  run_uci_loop(input, output);

  EXPECT_NE(output.str().find("bestmove"), std::string::npos);
}

TEST(uci, position_fen_parsed) {
  auto input = std::istringstream{
    "position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1\n"
    "go\n"
    "quit\n"
  };
  auto output = std::ostringstream{};

  run_uci_loop(input, output);

  EXPECT_NE(output.str().find("bestmove"), std::string::npos);
}

TEST(eval, startpos_is_zero) {
  const auto pos = position::from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1").value();

  EXPECT_EQ(evaluate(pos), 0);
}

TEST(eval, white_up_a_queen) {
  const auto pos = position::from_fen("4k3/8/8/8/8/8/8/3QK3 w - - 0 1").value();

  EXPECT_EQ(evaluate(pos), 900);
}

TEST(eval, black_up_a_rook_from_blacks_perspective) {
  const auto pos = position::from_fen("3rk3/8/8/8/8/8/8/4K3 b - - 0 1").value();

  EXPECT_EQ(evaluate(pos), 500);
}

TEST(search, finds_mate_in_one) {
  // White to move, Qa8# is mate
  auto pos = position::from_fen("k7/8/1K6/8/8/8/8/Q7 w - - 0 1").value();

  const auto result = search(pos, 3);

  EXPECT_EQ(result.best_move.from(), square::a1);
  EXPECT_EQ(result.best_move.to(), square::a8);
  EXPECT_GT(result.score, 50000);
}

TEST(search, captures_free_queen) {
  // White rook on a1, black queen on a8, nothing defending
  auto pos = position::from_fen("q3k3/8/8/8/8/8/8/R3K3 w - - 0 1").value();

  const auto result = search(pos, 2);

  EXPECT_EQ(result.best_move.from(), square::a1);
  EXPECT_EQ(result.best_move.to(), square::a8);
}

TEST(search, doesnt_hang_on_stalemate) {
  // Stalemate position
  auto pos = position::from_fen("k7/8/1Q6/8/8/8/8/4K3 b - - 0 1").value();

  const auto result = search(pos, 3);

  EXPECT_EQ(result.score, 0);
}

auto main(std::int32_t argc, char* argv[]) -> std::int32_t {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

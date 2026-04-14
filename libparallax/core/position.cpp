// SPDX-License-Identifier: MIT
#include <libparallax/core/position.hpp>

#include <cctype>
#include <charconv>
#include <iterator>
#include <utility>

#include <fmt/format.h>

#include <libparallax/core/attacks.hpp>

namespace parallax {

auto piece_from_char(const char character) -> std::expected<std::pair<color, piece>, std::string> {
  const auto piece_color = std::isupper(static_cast<unsigned char>(character)) ? color::white : color::black;

  switch (std::tolower(static_cast<unsigned char>(character))) {
    case 'p': return std::pair{piece_color, piece::pawn};
    case 'n': return std::pair{piece_color, piece::knight};
    case 'b': return std::pair{piece_color, piece::bishop};
    case 'r': return std::pair{piece_color, piece::rook};
    case 'q': return std::pair{piece_color, piece::queen};
    case 'k': return std::pair{piece_color, piece::king};
  }

  return std::unexpected{fmt::format("invalid piece character '{}'", character)};
}

auto char_from_piece(const color piece_color, const piece piece_type) -> char {
  constexpr auto piece_chars = std::array{'p', 'n', 'b', 'r', 'q', 'k'};

  const auto lowercase = piece_chars[static_cast<std::size_t>(piece_type)];

  return piece_color == color::white ? static_cast<char>(std::toupper(lowercase)) : lowercase;
}

auto split_fen(const std::string_view fen) -> std::expected<std::array<std::string_view, 6>, std::string> {
  auto fields = std::array<std::string_view, 6>{};
  auto field_index = 0uz;
  auto field_start = 0uz;

  for (auto index = 0uz; index <= fen.size(); ++index) {
    if (index == fen.size() || fen[index] == ' ') {
      if (index > field_start) {
        if (field_index >= 6) {
          return std::unexpected{"too many fen fields"};
        }

        fields[field_index++] = fen.substr(field_start, index - field_start);
      }

      field_start = index + 1;
    }
  }

  if (field_index != 6) {
    return std::unexpected{fmt::format("expected 6 fen fields, got {}", field_index)};
  }

  return fields;
}

constexpr auto castling_update_table = std::array<std::uint8_t, 64>{
  // a1=13, e1=12, h1=14, a8=7, e8=3, h8=11; all others = 15
  13, 15, 15, 15, 12, 15, 15, 14,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
   7, 15, 15, 15,  3, 15, 15, 11,
};

auto position::from_fen(const std::string_view fen) -> std::expected<position, std::string> {
  const auto fields_result = split_fen(fen);

  if (!fields_result) {
    return std::unexpected{fields_result.error()};
  }

  const auto& fields = *fields_result;

  auto result = position{};

  auto current_file = std::int32_t{0};
  auto current_rank = std::int32_t{7};

  for (const auto character : fields[0]) {
    if (character == '/') {
      if (current_file != 8) {
        return std::unexpected{fmt::format("rank {} has {} files, expected 8", current_rank, current_file)};
      }

      current_file = 0u;
      --current_rank;

      if (current_rank < 0) {
        return std::unexpected{"too many ranks in piece placement"};
      }

      continue;
    }

    if (character >= '1' && character <= '8') {
      current_file += character - '0';

      if (current_file > 8) {
        return std::unexpected{fmt::format("rank {} overflows past file h", current_rank)};
      }

      continue;
    }

    const auto parsed_piece = piece_from_char(character);

    if (!parsed_piece) {
      return std::unexpected{parsed_piece.error()};
    }

    if (current_file >= 8) {
      return std::unexpected{fmt::format("rank {} has too many pieces", current_rank)};
    }

    const auto [piece_color, piece_type] = *parsed_piece;
    const auto target_square = make_square(static_cast<std::uint32_t>(current_file), static_cast<std::uint32_t>(current_rank));
    const auto square_bit = bitboard{1} << static_cast<int>(target_square);

    result._pieces[static_cast<std::size_t>(piece_color)][static_cast<std::size_t>(piece_type)] |= square_bit;
    result._occupancy[static_cast<std::size_t>(piece_color)] |= square_bit;
    result._mailbox[static_cast<std::size_t>(target_square)] = piece_type;

    ++current_file;
  }

  if (current_rank != 0 || current_file != 8) {
    return std::unexpected{"piece placement did not cover all 64 squares"};
  }

  if (fields[1] == "w") {
    result._side_to_move = color::white;
  } else if (fields[1] == "b") {
    result._side_to_move = color::black;
  } else {
    return std::unexpected{fmt::format("invalid side to move '{}'", fields[1])};
  }

  result._castling_rights = 0u;

  if (fields[2] != "-") {
    for (const auto character : fields[2]) {
      switch (character) {
        case 'K': result._castling_rights |= castling::white_king; break;
        case 'Q': result._castling_rights |= castling::white_queen; break;
        case 'k': result._castling_rights |= castling::black_king; break;
        case 'q': result._castling_rights |= castling::black_queen; break;
        default: return std::unexpected{fmt::format("invalid castling character '{}'", character)};
      }
    }
  }

  if (fields[3] == "-") {
    result._ep_square = square::none;
  } else {
    if (fields[3].size() != 2 || fields[3][0] < 'a' || fields[3][0] > 'h' || fields[3][1] < '1' || fields[3][1] > '8') {
      return std::unexpected{fmt::format("invalid en passant square '{}'", fields[3])};
    }

    result._ep_square = make_square(static_cast<std::uint32_t>(fields[3][0] - 'a'), static_cast<std::uint32_t>(fields[3][1] - '1'));
  }

  auto halfmove_clock = std::int32_t{0};
  const auto halfmove_result = std::from_chars(fields[4].data(), fields[4].data() + fields[4].size(), halfmove_clock);

  if (halfmove_result.ec != std::errc{} || halfmove_result.ptr != fields[4].data() + fields[4].size() || halfmove_clock < 0 || halfmove_clock > 255) {
    return std::unexpected{fmt::format("invalid halfmove clock '{}'", fields[4])};
  }

  result._halfmove_clock = static_cast<std::uint8_t>(halfmove_clock);

  auto fullmove_number = 0u;
  const auto fullmove_result = std::from_chars(fields[5].data(), fields[5].data() + fields[5].size(), fullmove_number);

  if (fullmove_result.ec != std::errc{} || fullmove_result.ptr != fields[5].data() + fields[5].size() || fullmove_number < 1 || fullmove_number > 65535) {
    return std::unexpected{fmt::format("invalid fullmove number '{}'", fields[5])};
  }

  result._fullmove_number = static_cast<std::uint16_t>(fullmove_number);

  return result;
}

auto position::to_fen() const -> std::string {
  auto output = std::string{};

  for (auto current_rank = 7; current_rank >= 0; --current_rank) {
    auto empty_count = 0u;

    for (auto current_file = 0u; current_file < 8; ++current_file) {
      const auto current_square = make_square(current_file, static_cast<std::uint32_t>(current_rank));
      const auto piece_type = piece_at(current_square);

      if (piece_type == piece::none) {
        ++empty_count;
        continue;
      }

      if (empty_count > 0) {
        output += static_cast<char>('0' + empty_count);
        empty_count = 0u;
      }

      output += char_from_piece(color_at(current_square), piece_type);
    }

    if (empty_count > 0) {
      output += static_cast<char>('0' + empty_count);
    }

    if (current_rank > 0) {
      output += '/';
    }
  }

  output += _side_to_move == color::white ? " w " : " b ";

  if (_castling_rights == 0) {
    output += '-';
  } else {
    if (_castling_rights & castling::white_king) {
      output += 'K';
    }

    if (_castling_rights & castling::white_queen) {
      output += 'Q';
    }

    if (_castling_rights & castling::black_king) {
      output += 'k';
    }

    if (_castling_rights & castling::black_queen) {
      output += 'q';
    }
  }

  output += ' ';

  if (_ep_square == square::none) {
    output += '-';
  } else {
    output += static_cast<char>('a' + file_of(_ep_square));
    output += static_cast<char>('1' + rank_of(_ep_square));
  }

  fmt::format_to(std::back_inserter(output), " {} {}", _halfmove_clock, _fullmove_number);

  return output;
}

auto position::pieces(const color piece_color, const piece piece_type) const noexcept -> bitboard {
  return _pieces[static_cast<std::size_t>(piece_color)][static_cast<std::size_t>(piece_type)];
}

auto position::occupancy(const color piece_color) const noexcept -> bitboard {
  return _occupancy[static_cast<std::size_t>(piece_color)];
}

auto position::occupancy() const noexcept -> bitboard {
  return _occupancy[0] | _occupancy[1];
}

auto position::side_to_move() const noexcept -> color {
  return _side_to_move;
}

auto position::piece_at(const square target_square) const noexcept -> piece {
  return _mailbox[static_cast<std::size_t>(target_square)];
}

auto position::color_at(const square target_square) const noexcept -> color {
  const auto square_bit = square_to_bit(target_square);

  return (_occupancy[0] & square_bit) ? color::white : color::black;
}

auto position::zobrist() const noexcept -> std::uint64_t {
  return _zobrist;
}

auto position::place_piece(const color piece_color, const piece piece_type, const square target_square) noexcept -> void {
  const auto square_bit = square_to_bit(target_square);

  _pieces[static_cast<std::size_t>(piece_color)][static_cast<std::size_t>(piece_type)] |= square_bit;
  _occupancy[static_cast<std::size_t>(piece_color)] |= square_bit;
  _mailbox[static_cast<std::size_t>(target_square)] = piece_type;
}

auto position::remove_piece(const color piece_color, const piece piece_type, const square target_square) noexcept -> void {
  const auto square_bit = square_to_bit(target_square);

  _pieces[static_cast<std::size_t>(piece_color)][static_cast<std::size_t>(piece_type)] &= ~square_bit;
  _occupancy[static_cast<std::size_t>(piece_color)] &= ~square_bit;
  _mailbox[static_cast<std::size_t>(target_square)] = piece::none;
}

auto position::move_piece(const color piece_color, const piece piece_type, const square from_square, const square to_square) noexcept -> void {
  remove_piece(piece_color, piece_type, from_square);
  place_piece(piece_color, piece_type, to_square);
}

auto position::is_square_attacked(const square target_square, const color attacker_color) const noexcept -> bool {
  const auto attacker_index = static_cast<std::size_t>(attacker_color);
  const auto all_occupancy = occupancy();

  const auto defender_color = attacker_color == color::white ? color::black : color::white;

  if (pawn_attacks(defender_color, target_square) & _pieces[attacker_index][static_cast<std::size_t>(piece::pawn)]) {
    return true;
  }

  if (knight_attacks(target_square) & _pieces[attacker_index][static_cast<std::size_t>(piece::knight)]) {
    return true;
  }

  if (king_attacks(target_square) & _pieces[attacker_index][static_cast<std::size_t>(piece::king)]) {
    return true;
  }

  const auto bishops_and_queens
    = _pieces[attacker_index][static_cast<std::size_t>(piece::bishop)]
    | _pieces[attacker_index][static_cast<std::size_t>(piece::queen)];

  if (bishop_attacks(target_square, all_occupancy) & bishops_and_queens) {
    return true;
  }

  const auto rooks_and_queens
    = _pieces[attacker_index][static_cast<std::size_t>(piece::rook)]
    | _pieces[attacker_index][static_cast<std::size_t>(piece::queen)];

  if (rook_attacks(target_square, all_occupancy) & rooks_and_queens) {
    return true;
  }

  return false;
}

auto position::en_passant_square() const noexcept -> square {
  return _ep_square;
}

auto position::castling_rights() const noexcept -> std::uint8_t {
  return _castling_rights;
}

auto position::make_move(const move played_move) -> void {
  const auto from_square = played_move.from();
  const auto to_square = played_move.to();
  const auto move_flags = played_move.flag();
  const auto mover_color = _side_to_move;
  const auto enemy_color = mover_color == color::white ? color::black : color::white;
  const auto mover_piece = _mailbox[static_cast<std::size_t>(from_square)];

  auto undo = undo_info{};
  undo.played = played_move;
  undo.captured = piece::none;
  undo.ep_square = _ep_square;
  undo.castling_rights = _castling_rights;
  undo.halfmove_clock = _halfmove_clock;
  undo.zobrist = _zobrist;

  _ep_square = square::none;
  ++_halfmove_clock;

  if (mover_piece == piece::pawn) {
    _halfmove_clock = 0u;
  }

  switch (move_flags) {
    case move_flag::quiet: {
      move_piece(mover_color, mover_piece, from_square, to_square);
      break;
    }
    case move_flag::double_push: {
      move_piece(mover_color, mover_piece, from_square, to_square);
      const auto ep_rank_offset = mover_color == color::white ? -8 : 8;
      _ep_square = static_cast<square>(static_cast<std::int32_t>(to_square) + ep_rank_offset);
      break;
    }
    case move_flag::capture: {
      const auto captured_piece = _mailbox[static_cast<std::size_t>(to_square)];
      undo.captured = captured_piece;
      remove_piece(enemy_color, captured_piece, to_square);
      move_piece(mover_color, mover_piece, from_square, to_square);
      _halfmove_clock = 0u;
      break;
    }
    case move_flag::en_passant: {
      const auto captured_pawn_square = static_cast<square>(static_cast<std::int32_t>(to_square) + (mover_color == color::white ? -8 : 8));
      undo.captured = piece::pawn;
      remove_piece(enemy_color, piece::pawn, captured_pawn_square);
      move_piece(mover_color, piece::pawn, from_square, to_square);
      _halfmove_clock = 0u;
      break;
    }
    case move_flag::castle_king: {
      move_piece(mover_color, piece::king, from_square, to_square);
      const auto rook_from = mover_color == color::white ? square::h1 : square::h8;
      const auto rook_to = mover_color == color::white ? square::f1 : square::f8;
      move_piece(mover_color, piece::rook, rook_from, rook_to);
      break;
    }
    case move_flag::castle_queen: {
      move_piece(mover_color, piece::king, from_square, to_square);
      const auto rook_from = mover_color == color::white ? square::a1 : square::a8;
      const auto rook_to = mover_color == color::white ? square::d1 : square::d8;
      move_piece(mover_color, piece::rook, rook_from, rook_to);
      break;
    }
    case move_flag::promo_knight:
    case move_flag::promo_bishop:
    case move_flag::promo_rook:
    case move_flag::promo_queen: {
      const auto promoted_to = static_cast<piece>(static_cast<std::uint8_t>(move_flags) - static_cast<std::uint8_t>(move_flag::promo_knight) + static_cast<std::uint8_t>(piece::knight));
      remove_piece(mover_color, piece::pawn, from_square);
      place_piece(mover_color, promoted_to, to_square);
      _halfmove_clock = 0u;
      break;
    }
    case move_flag::promo_capture_knight:
    case move_flag::promo_capture_bishop:
    case move_flag::promo_capture_rook:
    case move_flag::promo_capture_queen: {
      const auto captured_piece = _mailbox[static_cast<std::size_t>(to_square)];
      undo.captured = captured_piece;
      const auto promoted_to = static_cast<piece>(static_cast<std::uint8_t>(move_flags) - static_cast<std::uint8_t>(move_flag::promo_capture_knight) + static_cast<std::uint8_t>(piece::knight));
      remove_piece(enemy_color, captured_piece, to_square);
      remove_piece(mover_color, piece::pawn, from_square);
      place_piece(mover_color, promoted_to, to_square);
      _halfmove_clock = 0u;
      break;
    }

  }

  _castling_rights &= castling_update_table[static_cast<std::size_t>(from_square)];
  _castling_rights &= castling_update_table[static_cast<std::size_t>(to_square)];

  if (mover_color == color::black) {
    ++_fullmove_number;
  }

  _side_to_move = enemy_color;
  _history.push_back(undo);
}

auto position::unmake_move() -> void {
  const auto undo = _history.back();
  _history.pop_back();

  _side_to_move = _side_to_move == color::white ? color::black : color::white;

  if (_side_to_move == color::black) {
    --_fullmove_number;
  }

  const auto played_move = undo.played;
  const auto from_square = played_move.from();
  const auto to_square = played_move.to();
  const auto move_flags = played_move.flag();
  const auto mover_color = _side_to_move;
  const auto enemy_color = mover_color == color::white ? color::black : color::white;

  switch (move_flags) {
    case move_flag::quiet:
    case move_flag::double_push: {
      const auto mover_piece = _mailbox[static_cast<std::size_t>(to_square)];
      move_piece(mover_color, mover_piece, to_square, from_square);
      break;
    }
    case move_flag::capture: {
      const auto mover_piece = _mailbox[static_cast<std::size_t>(to_square)];
      move_piece(mover_color, mover_piece, to_square, from_square);
      place_piece(enemy_color, undo.captured, to_square);
      break;
    }
    case move_flag::en_passant: {
      move_piece(mover_color, piece::pawn, to_square, from_square);
      const auto captured_pawn_square = static_cast<square>(static_cast<std::int32_t>(to_square) + (mover_color == color::white ? -8 : 8));
      place_piece(enemy_color, piece::pawn, captured_pawn_square);
      break;
    }
    case move_flag::castle_king: {
      move_piece(mover_color, piece::king, to_square, from_square);
      const auto rook_from = mover_color == color::white ? square::h1 : square::h8;
      const auto rook_to = mover_color == color::white ? square::f1 : square::f8;
      move_piece(mover_color, piece::rook, rook_to, rook_from);
      break;
    }
    case move_flag::castle_queen: {
      move_piece(mover_color, piece::king, to_square, from_square);
      const auto rook_from = mover_color == color::white ? square::a1 : square::a8;
      const auto rook_to = mover_color == color::white ? square::d1 : square::d8;
      move_piece(mover_color, piece::rook, rook_to, rook_from);
      break;
    }
    case move_flag::promo_knight:
    case move_flag::promo_bishop:
    case move_flag::promo_rook:
    case move_flag::promo_queen: {
      const auto promoted_to = _mailbox[static_cast<std::size_t>(to_square)];
      remove_piece(mover_color, promoted_to, to_square);
      place_piece(mover_color, piece::pawn, from_square);
      break;
    }
    case move_flag::promo_capture_knight:
    case move_flag::promo_capture_bishop:
    case move_flag::promo_capture_rook:
    case move_flag::promo_capture_queen: {
      const auto promoted_to = _mailbox[static_cast<std::size_t>(to_square)];
      remove_piece(mover_color, promoted_to, to_square);
      place_piece(mover_color, piece::pawn, from_square);
      place_piece(enemy_color, undo.captured, to_square);
      break;
    }

  }

  _ep_square = undo.ep_square;
  _castling_rights = undo.castling_rights;
  _halfmove_clock = undo.halfmove_clock;
  _zobrist = undo.zobrist;
}

auto position::in_check() const noexcept -> bool {
  const auto king_board = _pieces[static_cast<std::size_t>(_side_to_move)][static_cast<std::size_t>(piece::king)];

  if (king_board == 0) {
    return false;
  }

  const auto king_square = lsb(king_board);
  const auto attacker_color = _side_to_move == color::white ? color::black : color::white;

  return is_square_attacked(king_square, attacker_color);
}

} // namespace parallax

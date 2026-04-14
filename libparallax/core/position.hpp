// SPDX-License-Identifier: MIT
#ifndef LIBPARALLAX_CORE_POSITION_HPP_
#define LIBPARALLAX_CORE_POSITION_HPP_

#include <cstdint>

#include <array>
#include <expected>
#include <string>
#include <string_view>
#include <vector>

#include <libparallax/core/bitboard.hpp>
#include <libparallax/core/move.hpp>
#include <libparallax/core/zobrist.hpp>

namespace parallax {

enum class color : std::uint8_t {
  white,
  black
}; // enum class color

enum class piece : std::uint8_t {
  pawn,
  knight,
  bishop,
  rook,
  queen,
  king,
  none
}; // enum class piece

namespace castling {

inline constexpr auto white_king = std::uint8_t{1};
inline constexpr auto white_queen = std::uint8_t{2};
inline constexpr auto black_king = std::uint8_t{4};
inline constexpr auto black_queen = std::uint8_t{8};

} // namespace castling

struct undo_info {
  move played;
  piece captured;
  square ep_square;
  std::uint8_t castling_rights;
  std::uint8_t halfmove_clock;
  std::uint64_t zobrist;
}; // struct undo_info

class position {

public:

  position() noexcept {
    _mailbox.fill(piece::none);
    _history.reserve(256);
  }

  static auto from_fen(const std::string_view fen) -> std::expected<position, std::string>;

  auto to_fen() const -> std::string;

  auto pieces(const color piece_color, const piece piece_type) const noexcept -> bitboard;

  auto occupancy(const color piece_color) const noexcept -> bitboard;

  auto occupancy() const noexcept -> bitboard;

  auto side_to_move() const noexcept -> color;

  auto piece_at(const square target_square) const noexcept -> piece;

  auto color_at(const square target_square) const noexcept -> color;

  auto zobrist() const noexcept -> std::uint64_t;

  auto make_move(const move played_move) -> void;

  auto unmake_move() -> void;

  auto in_check() const noexcept -> bool;

  auto en_passant_square() const noexcept -> square;

  auto castling_rights() const noexcept -> std::uint8_t;

  auto is_square_attacked(const square target_square, const color attacker_color) const noexcept -> bool;

  auto debug_recomputed_zobrist() const noexcept -> std::uint64_t;

private:

  auto place_piece(const color piece_color, const piece piece_type, const square target_square) noexcept -> void;

  auto remove_piece(const color piece_color, const piece piece_type, const square target_square) noexcept -> void;

  auto move_piece(const color piece_color, const piece piece_type, const square from_square, const square to_square) noexcept -> void;

  auto recompute_zobrist() noexcept -> void;

  std::array<std::array<bitboard, 6>, 2> _pieces{};
  std::array<bitboard, 2> _occupancy{};
  std::array<piece, 64> _mailbox{};
  color _side_to_move{color::white};
  square _ep_square{square::none};
  std::uint8_t _castling_rights{0};
  std::uint8_t _halfmove_clock{0};
  std::uint16_t _fullmove_number{1};
  std::uint64_t _zobrist{0};
  std::vector<undo_info> _history{};

}; // class position

} // namespace parallax

#endif // LIBPARALLAX_CORE_POSITION_HPP_

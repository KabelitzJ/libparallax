// SPDX-License-Identifier: MIT
#ifndef LIBPARALLAY_CORE_POSITION_HPP_
#define LIBPARALLAY_CORE_POSITION_HPP_

#include <cstdint>

#include <expected>
#include <array>
#include <vector>

#include <libparallax/core/move.hpp>

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

using bitboard = std::uint64_t;

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

  static auto from_fen(std::string_view fen) -> std::expected<position, std::string>;

  auto to_fen() const -> std::string;

  auto pieces(const color color, const piece piece) const noexcept -> bitboard;

  auto occupancy(const color color) const noexcept -> bitboard;

  auto occupancy() const noexcept -> bitboard;

  auto side_to_move() const noexcept -> color;

  auto piece_at(const square square) const noexcept -> piece;

  auto color_at(const square square) const noexcept -> color;

  auto zobrist() const noexcept -> std::uint64_t;

  auto make_move(const move move) -> void;

  auto unmake_move() -> void;

  auto in_check() const noexcept -> bool;

private:

  std::array<std::array<bitboard, 6>, 2> _pieces{};
  std::array<bitboard, 2> _occupancy{};
  color _side_to_move{color::white};
  square _ep_square{square::none};
  std::uint8_t _castling_rights{0};
  std::uint8_t _halfmove_clock{0};
  std::uint16_t _fullmove_number{1};
  std::uint64_t _zobrist{0};
  std::vector<undo_info> _history{};

}; // class position

} // namespace parallax

#endif // LIBPARALLAY_CORE_POSITION_HPP_
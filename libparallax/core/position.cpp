// SPDX-License-Identifier: MIT
#include <libparallax/core/position.hpp>

namespace parallax {

auto position::from_fen(std::string_view fen) -> std::expected<position, std::string> {
  return position{};
}

auto position::to_fen() const -> std::string {
  return "";
}

auto position::pieces(const color color, const piece piece) const noexcept -> bitboard {
  return _pieces[static_cast<std::size_t>(color)][static_cast<std::size_t>(piece)];
}

auto position::occupancy(const color color) const noexcept -> bitboard {
  return _occupancy[static_cast<std::size_t>(color)];
}

auto position::occupancy() const noexcept -> bitboard {
  return _occupancy[0] | _occupancy[1];
}

auto position::side_to_move() const noexcept -> color {
  return _side_to_move;
}

auto position::piece_at(const square square) const noexcept -> piece {
  return piece::king;
}

auto position::color_at(const square square) const noexcept -> color {
  return color::white;
}

auto position::zobrist() const noexcept -> std::uint64_t {
  return _zobrist;
}

auto position::make_move(const move move) -> void {

}

auto position::unmake_move() -> void {

}

auto position::in_check() const noexcept -> bool {
  return false;
}

} // namespace parallax
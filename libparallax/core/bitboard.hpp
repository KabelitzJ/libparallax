// SPDX-License-Identifier: MIT
#ifndef LIBPARALLAX_CORE_BITBOARD_HPP_
#define LIBPARALLAX_CORE_BITBOARD_HPP_

#include <cstdint>

#include <bit>

#include <libparallax/core/move.hpp>

namespace parallax {

using bitboard = std::uint64_t;

inline constexpr auto file_a = bitboard{0x0101010101010101ULL};
inline constexpr auto file_b = file_a << 1;
inline constexpr auto file_c = file_a << 2;
inline constexpr auto file_d = file_a << 3;
inline constexpr auto file_e = file_a << 4;
inline constexpr auto file_f = file_a << 5;
inline constexpr auto file_g = file_a << 6;
inline constexpr auto file_h = file_a << 7;

inline constexpr auto rank_1 = bitboard{0x00000000000000FFULL};
inline constexpr auto rank_2 = rank_1 << (8 * 1);
inline constexpr auto rank_3 = rank_1 << (8 * 2);
inline constexpr auto rank_4 = rank_1 << (8 * 3);
inline constexpr auto rank_5 = rank_1 << (8 * 4);
inline constexpr auto rank_6 = rank_1 << (8 * 5);
inline constexpr auto rank_7 = rank_1 << (8 * 6);
inline constexpr auto rank_8 = rank_1 << (8 * 7);

constexpr auto square_to_bit(const square target_square) noexcept -> bitboard {
  return bitboard{1} << static_cast<std::int32_t>(target_square);
}

constexpr auto popcount(const bitboard board) noexcept -> std::int32_t {
  return std::popcount(board);
}

constexpr auto lsb(const bitboard board) noexcept -> square {
  return static_cast<square>(std::countr_zero(board));
}

constexpr auto msb(const bitboard board) noexcept -> square {
  return static_cast<square>(63 - std::countl_zero(board));
}

constexpr auto pop_lsb(bitboard& board) noexcept -> square {
  const auto index = lsb(board);
  board &= board - 1;
  return index;
}

constexpr auto shift_north(const bitboard board) noexcept -> bitboard {
  return board << 8;
}

constexpr auto shift_south(const bitboard board) noexcept -> bitboard {
  return board >> 8;
}

constexpr auto shift_east(const bitboard board) noexcept -> bitboard {
  return (board & ~file_h) << 1;
}

constexpr auto shift_west(const bitboard board) noexcept -> bitboard {
  return (board & ~file_a) >> 1;
}

constexpr auto shift_north_east(const bitboard board) noexcept -> bitboard {
  return (board & ~file_h) << 9;
}

constexpr auto shift_north_west(const bitboard board) noexcept -> bitboard {
  return (board & ~file_a) << 7;
}

constexpr auto shift_south_east(const bitboard board) noexcept -> bitboard {
  return (board & ~file_h) >> 7;
}

constexpr auto shift_south_west(const bitboard board) noexcept -> bitboard {
  return (board & ~file_a) >> 9;
}

class bitboard_iterator {

public:

  constexpr explicit bitboard_iterator(const bitboard board) noexcept
  : _remaining{board} { }

  constexpr auto operator*() const noexcept -> square {
    return lsb(_remaining);
  }

  constexpr auto operator++() noexcept -> bitboard_iterator& {
    _remaining &= _remaining - 1;
    return *this;
  }

  constexpr auto operator!=(const bitboard_iterator& other) const noexcept -> bool {
    return _remaining != other._remaining;
  }

private:

  bitboard _remaining{0};

}; // class bitboard_iterator

class bitboard_range {

public:

  constexpr explicit bitboard_range(const bitboard board) noexcept
  : _board{board} { }

  constexpr auto begin() const noexcept -> bitboard_iterator {
    return bitboard_iterator{_board};
  }

  constexpr auto end() const noexcept -> bitboard_iterator {
    return bitboard_iterator{0};
  }

private:

  bitboard _board{0};

}; // class bitboard_range

constexpr auto squares_of(const bitboard board) noexcept -> bitboard_range {
  return bitboard_range{board};
}

} // namespace parallax

#endif // LIBPARALLAX_CORE_BITBOARD_HPP_

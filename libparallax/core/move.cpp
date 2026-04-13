// SPDX-License-Identifier: MIT
#include <libparallax/core/move.hpp>

namespace parallax {

constexpr move::move(square from, square to, move_flag flag) noexcept
: _data{static_cast<std::uint16_t>((static_cast<std::uint16_t>(from) & 0x3F) | ((static_cast<std::uint16_t>(to) & 0x3F) << 6) | ((static_cast<std::uint16_t>(flag) & 0x0F) << 12))} { }

constexpr auto move::from() const noexcept -> square {
  return static_cast<square>(_data & 0x3F);
}

constexpr auto move::to() const noexcept -> square {
  return static_cast<square>((_data >> 6) & 0x3F);
}

constexpr auto move::flag() const noexcept -> move_flag {
  return static_cast<move_flag>((_data >> 12) & 0x0F);
}

constexpr auto move::is_capture() const noexcept -> bool {
  return (_data >> 12) & 0x4;
}

constexpr auto move::is_promotion() const noexcept -> bool {
  return (_data >> 12) & 0x8;
}

constexpr auto move::raw() const noexcept -> std::uint16_t {
  return _data;
}

} // namespace parallax


template<typename FormatContext>
auto fmt::formatter<parallax::move>::format(const parallax::move& move, FormatContext& context) const noexcept -> decltype(context.out()) {
  auto buffer = std::array<char, 6>{};
  auto from = static_cast<std::uint8_t>(move.from());
  auto to = static_cast<std::uint8_t>(move.to());

  buffer[0] = 'a' + (from & 7);
  buffer[1] = '1' + (from >> 3);
  buffer[2] = 'a' + (to & 7);
  buffer[3] = '1' + (to >> 3);

  auto length = 4uz;

  if (move.is_promotion()) {
    constexpr auto promotion = std::array{'n', 'b', 'r', 'q'};

    buffer[4] = promotion[static_cast<std::uint8_t>(move.flag()) & 0x3];
    length = 5;
  }

  return base::format(std::string_view{buffer.data(), length}, context);
}

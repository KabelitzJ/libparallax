// SPDX-License-Identifier: MIT
#ifndef PARALLAX_SEARCH_HPP_
#define PARALLAX_SEARCH_HPP_

#include <libparallax/core/move.hpp>
#include <libparallax/core/position.hpp>

namespace parallax {

struct search_result {
  move best_move;
  std::int32_t score;
  std::uint64_t nodes;
}; // struct search_result

auto search(position& current_position, const std::int32_t depth) -> search_result;

} // namespace parallax

#endif // PARALLAX_SEARCH_HPP_
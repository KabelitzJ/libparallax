// SPDX-License-Identifier: MIT
#ifndef PARALLAX_SEARCH_HPP_
#define PARALLAX_SEARCH_HPP_

#include <libparallax/core/move.hpp>
#include <libparallax/core/position.hpp>

namespace parallax {

struct search_result {
  move best_move;
  int score;
  std::uint64_t nodes;
};

auto search(position& current_position, const int depth) -> search_result;

} // namespace parallax

#endif // PARALLAX_SEARCH_HPP_
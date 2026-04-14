// SPDX-License-Identifier: MIT
#ifndef PARALLAX_SEARCH_HPP_
#define PARALLAX_SEARCH_HPP_

#include <chrono>

#include <libparallax/core/move.hpp>
#include <libparallax/core/position.hpp>

#include <libparallax/search/transposition_table.hpp>

namespace parallax {

struct search_limits {
  std::chrono::milliseconds max_time{0};
  std::int32_t max_depth{64};
}; // struct search_limits

struct search_result {
  move best_move;
  std::int32_t score;
  std::int32_t depth;
  std::uint64_t nodes;
  std::chrono::milliseconds elapsed;
}; // struct search_result

auto clear_transposition_table() -> void;

auto search(position& current_position, const search_limits& limits) -> search_result;

} // namespace parallax

#endif // PARALLAX_SEARCH_HPP_
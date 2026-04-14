// SPDX-License-Identifier: MIT
#ifndef PARALLAX_PERFT_HPP_
#define PARALLAX_PERFT_HPP_

#include <fmt/printf.h>

#include <libparallax/core/move.hpp>
#include <libparallax/core/position.hpp>

namespace parallax {

auto generate_legal_moves(const position& position) -> std::vector<move>;

inline auto perft(position& position, int depth) -> std::uint64_t {
  if (depth == 0) {
    return 1;
  }
  auto moves = generate_legal_moves(position);
  if (depth == 1) {
    return moves.size();
  }
  auto nodes = std::uint64_t{0};
  for (auto m : moves) {
    position.make_move(m);
    nodes += perft(position, depth - 1);
    position.unmake_move();
  }
  return nodes;
}

inline auto perft_divide(position& position, int depth) -> std::uint64_t {
  auto moves = generate_legal_moves(position);
  auto total = std::uint64_t{0};
  for (auto m : moves) {
    position.make_move(m);
    auto nodes = depth <= 1 ? std::uint64_t{1} : perft(position, depth - 1);
    position.unmake_move();
    fmt::println("{}: {}", m, nodes);
    total += nodes;
  }
  fmt::println("nodes: {}", total);
  return total;
}

} // namespace parallax

#endif // PARALLAX_PERFT_HPP_
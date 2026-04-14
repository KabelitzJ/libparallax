// SPDX-License-Identifier: MIT
#include <libparallax/search/search.hpp>

#include <algorithm>
#include <limits>

#include <libparallax/eval/eval.hpp>
#include <libparallax/movegen/movegen.hpp>

namespace parallax {

constexpr auto infinity_score = 1'000'000;
constexpr auto mate_score = 100'000;

auto negamax(position& current_position, const std::int32_t depth, std::int32_t alpha, const std::int32_t beta, std::uint64_t& node_count) -> std::int32_t {
  ++node_count;

  if (depth == 0) {
    return evaluate(current_position);
  }

  const auto legal_moves = generate_legal_moves(current_position);

  if (legal_moves.empty()) {
    if (current_position.in_check()) {
      return -mate_score + (1000 - depth);
    }

    return 0;
  }

  auto best_score = -infinity_score;

  for (const auto candidate : legal_moves) {
    current_position.make_move(candidate);
    const auto child_score = -negamax(current_position, depth - 1, -beta, -alpha, node_count);
    current_position.unmake_move();

    if (child_score > best_score) {
      best_score = child_score;
    }

    if (best_score > alpha) {
      alpha = best_score;
    }

    if (alpha >= beta) {
      break;
    }
  }

  return best_score;
}

auto search(position& current_position, const std::int32_t depth) -> search_result {
  auto result = search_result{};
  result.nodes = 0u;

  const auto legal_moves = generate_legal_moves(current_position);

  if (legal_moves.empty()) {
    result.score = current_position.in_check() ? -mate_score : 0;
    return result;
  }

  auto alpha = -infinity_score;
  const auto beta = infinity_score;

  result.best_move = legal_moves[0];
  result.score = -infinity_score;

  for (const auto candidate : legal_moves) {
    current_position.make_move(candidate);
    const auto child_score = -negamax(current_position, depth - 1, -beta, -alpha, result.nodes);
    current_position.unmake_move();

    if (child_score > result.score) {
      result.score = child_score;
      result.best_move = candidate;
    }

    if (result.score > alpha) {
      alpha = result.score;
    }
  }

  return result;
}

} // namespace parallax

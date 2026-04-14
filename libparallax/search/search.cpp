// SPDX-License-Identifier: MIT
#include <libparallax/search/search.hpp>

#include <algorithm>
#include <chrono>
#include <limits>
#include <vector>

#include <libparallax/eval/eval.hpp>
#include <libparallax/movegen/movegen.hpp>

namespace parallax {

constexpr auto infinity_score = std::int32_t{1'000'000};
constexpr auto mate_score = std::int32_t{100'000};

constexpr auto mvv_lva_victim_values = std::array<std::int32_t, 7>{
  100,    // pawn
  320,    // knight
  330,    // bishop
  500,    // rook
  900,    // queen
  10000,  // king
  0       // none
};

struct search_context {
  std::chrono::steady_clock::time_point start_time;
  std::chrono::milliseconds time_budget;
  std::uint64_t nodes;
  bool stopped;
}; // struct search_context

auto score_move(const position& current_position, const move candidate) -> std::int32_t {
  if (candidate.is_capture()) {
    const auto victim = candidate.flag() == move_flag::en_passant ? piece::pawn : current_position.piece_at(candidate.to());
    const auto attacker = current_position.piece_at(candidate.from());

    return 1'000'000 + mvv_lva_victim_values[static_cast<std::size_t>(victim)] * 10 - mvv_lva_victim_values[static_cast<std::size_t>(attacker)];
  }

  if (candidate.is_promotion()) {
    return 900'000;
  }

  return 0;
}

auto order_moves(const position& current_position, std::vector<move>& moves) -> void {
  std::sort(moves.begin(), moves.end(), [&](const move lhs, const move rhs) {
    return score_move(current_position, lhs) > score_move(current_position, rhs);
  });
}

auto time_up(search_context& context) -> bool {
  if (context.stopped) {
    return true;
  }

  if (context.time_budget.count() == 0) {
    return false;
  }

  if ((context.nodes & 2047u) != 0) {
    return false;
  }

  const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - context.start_time);

  if (elapsed >= context.time_budget) {
    context.stopped = true;
    return true;
  }

  return false;
}

auto quiescence(position& current_position, std::int32_t alpha, const std::int32_t beta, search_context& context) -> std::int32_t {
  ++context.nodes;

  if (time_up(context)) {
    return 0;
  }

  const auto stand_pat = evaluate(current_position);

  if (stand_pat >= beta) {
    return beta;
  }

  if (stand_pat > alpha) {
    alpha = stand_pat;
  }

  auto legal_moves = generate_legal_moves(current_position);

  order_moves(current_position, legal_moves);

  for (const auto candidate : legal_moves) {
    if (!candidate.is_capture() && !candidate.is_promotion()) {
      continue;
    }

    current_position.make_move(candidate);
    const auto child_score = -quiescence(current_position, -beta, -alpha, context);
    current_position.unmake_move();

    if (context.stopped) {
      return 0;
    }

    if (child_score >= beta) {
      return beta;
    }

    if (child_score > alpha) {
      alpha = child_score;
    }
  }

  return alpha;
}

auto negamax(position& current_position, const std::int32_t depth, std::int32_t alpha, const std::int32_t beta, search_context& context) -> std::int32_t {
  ++context.nodes;

  if (time_up(context)) {
    return 0;
  }

  if (depth == 0) {
    return quiescence(current_position, alpha, beta, context);
  }

  auto legal_moves = generate_legal_moves(current_position);

  if (legal_moves.empty()) {
    if (current_position.in_check()) {
      return -mate_score + (1000 - depth);
    }

    return 0;
  }

  order_moves(current_position, legal_moves);

  auto best_score = -infinity_score;

  for (const auto candidate : legal_moves) {
    current_position.make_move(candidate);
    const auto child_score = -negamax(current_position, depth - 1, -beta, -alpha, context);
    current_position.unmake_move();

    if (context.stopped) {
      return 0;
    }

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

auto search(position& current_position, const search_limits& limits) -> search_result {
  auto context = search_context{};
  context.start_time = std::chrono::steady_clock::now();
  context.time_budget = limits.max_time;
  context.nodes = 0;
  context.stopped = false;

  auto result = search_result{};

  auto root_moves = generate_legal_moves(current_position);

  if (root_moves.empty()) {
    result.score = current_position.in_check() ? -mate_score : 0;
    return result;
  }

  order_moves(current_position, root_moves);
  result.best_move = root_moves[0];

  for (auto current_depth = std::int32_t{1}; current_depth <= limits.max_depth; ++current_depth) {
    auto iteration_best_move = move{};
    auto iteration_best_score = -infinity_score;
    auto alpha = -infinity_score;
    const auto beta = infinity_score;

    for (const auto candidate : root_moves) {
      current_position.make_move(candidate);
      const auto child_score = -negamax(current_position, current_depth - 1, -beta, -alpha, context);
      current_position.unmake_move();

      if (context.stopped) {
        break;
      }

      if (child_score > iteration_best_score) {
        iteration_best_score = child_score;
        iteration_best_move = candidate;
      }

      if (iteration_best_score > alpha) {
        alpha = iteration_best_score;
      }
    }

    if (context.stopped) {
      break;
    }

    result.best_move = iteration_best_move;
    result.score = iteration_best_score;
    result.depth = current_depth;
    result.nodes = context.nodes;
    result.elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - context.start_time);
  }

  result.nodes = context.nodes;
  result.elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - context.start_time);

  return result;
}

} // namespace parallax

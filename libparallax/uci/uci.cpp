// SPDX-License-Identifier: MIT
#include <libparallax/uci/uci.hpp>

#include <algorithm>
#include <charconv>
#include <chrono>
#include <iostream>
#include <random>
#include <sstream>
#include <string_view>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <libparallax/core/move.hpp>
#include <libparallax/core/position.hpp>

#include <libparallax/movegen/movegen.hpp>

#include <libparallax/search/search.hpp>

namespace parallax {

constexpr auto engine_name = std::string_view{"parallax 0.1"};
constexpr auto engine_author = std::string_view{"jonas"};

constexpr auto startpos_fen = std::string_view{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"};

auto parse_square(const std::string_view text) -> square {
  if (text.size() < 2) {
    return square::none;
  }

  const auto file = text[0] - 'a';
  const auto rank = text[1] - '1';

  if (file < 0 || file > 7 || rank < 0 || rank > 7) {
    return square::none;
  }

  return make_square(static_cast<std::uint32_t>(file), static_cast<std::uint32_t>(rank));
}

auto find_legal_move(const position& current_position, const std::string_view move_text) -> move {
  if (move_text.size() < 4) {
    return move{};
  }

  const auto from_square = parse_square(move_text.substr(0, 2));
  const auto to_square = parse_square(move_text.substr(2, 2));

  if (from_square == square::none || to_square == square::none) {
    return move{};
  }

  const auto promotion_char = move_text.size() >= 5 ? move_text[4] : '\0';

  const auto legal_moves = generate_legal_moves(current_position);

  for (const auto candidate : legal_moves) {
    if (candidate.from() != from_square || candidate.to() != to_square) {
      continue;
    }

    if (promotion_char == '\0') {
      if (!candidate.is_promotion()) {
        return candidate;
      }
      continue;
    }

    if (!candidate.is_promotion()) {
      continue;
    }

    const auto flag = candidate.flag();
    const auto is_capture_promo = candidate.is_capture();

    const auto matches
      = (promotion_char == 'q' && (flag == move_flag::promo_queen || flag == move_flag::promo_capture_queen))
      || (promotion_char == 'r' && (flag == move_flag::promo_rook || flag == move_flag::promo_capture_rook))
      || (promotion_char == 'b' && (flag == move_flag::promo_bishop || flag == move_flag::promo_capture_bishop))
      || (promotion_char == 'n' && (flag == move_flag::promo_knight || flag == move_flag::promo_capture_knight));

    (void)is_capture_promo;

    if (matches) {
      return candidate;
    }
  }

  return move{};
}

auto split_tokens(const std::string_view line) -> std::vector<std::string_view> {
  auto tokens = std::vector<std::string_view>{};

  auto cursor = 0uz;

  while (cursor < line.size()) {
    while (cursor < line.size() && (line[cursor] == ' ' || line[cursor] == '\t' || line[cursor] == '\r')) {
      ++cursor;
    }

    if (cursor >= line.size()) {
      break;
    }

    const auto token_start = cursor;

    while (cursor < line.size() && line[cursor] != ' ' && line[cursor] != '\t' && line[cursor] != '\r') {
      ++cursor;
    }

    tokens.emplace_back(line.substr(token_start, cursor - token_start));
  }

  return tokens;
}

auto handle_position_command(const std::vector<std::string_view>& tokens, position& current_position) -> void {
  if (tokens.size() < 2) {
    return;
  }

  auto moves_index = 0uz;

  if (tokens[1] == "startpos") {
    auto parsed = position::from_fen(startpos_fen);

    if (!parsed) {
      return;
    }

    current_position = std::move(*parsed);
    moves_index = 2;
  } else if (tokens[1] == "fen") {
    if (tokens.size() < 8) {
      return;
    }

    auto fen_string = std::string{};
    fen_string.reserve(80);

    for (auto index = 2uz; index < 8; ++index) {
      if (index > 2) {
        fen_string += ' ';
      }
      fen_string += tokens[index];
    }

    auto parsed = position::from_fen(fen_string);

    if (!parsed) {
      return;
    }

    current_position = std::move(*parsed);
    moves_index = 8;
  } else {
    return;
  }

  if (moves_index >= tokens.size() || tokens[moves_index] != "moves") {
    return;
  }

  for (auto index = moves_index + 1; index < tokens.size(); ++index) {
    const auto played = find_legal_move(current_position, tokens[index]);

    if (played == move{}) {
      return;
    }

    current_position.make_move(played);
  }
}

auto parse_go_limits(const std::vector<std::string_view>& tokens, const color mover_color) -> search_limits {
  auto limits = search_limits{};
  limits.max_depth = 64;

  auto wtime = 0;
  auto btime = 0;
  auto winc = 0;
  auto binc = 0;
  auto movetime = 0;
  auto depth = 0;

  for (auto index = 1uz; index + 1 < tokens.size(); index += 2) {
    auto value = 0;
    std::from_chars(tokens[index + 1].data(), tokens[index + 1].data() + tokens[index + 1].size(), value);

    if (tokens[index] == "wtime") {
      wtime = value;
    } else if (tokens[index] == "btime") {
      btime = value;
    } else if (tokens[index] == "winc") {
      winc = value;
    } else if (tokens[index] == "binc") {
      binc = value;
    } else if (tokens[index] == "movetime") {
      movetime = value;
    } else if (tokens[index] == "depth") {
      depth = value;
    }
  }

  if (depth > 0) {
    limits.max_depth = depth;
    limits.max_time = std::chrono::milliseconds{0};

    return limits;
  }

  if (movetime > 0) {
    limits.max_time = std::chrono::milliseconds{movetime};

    return limits;
  }

  const auto own_time = mover_color == color::white ? wtime : btime;
  const auto own_inc = mover_color == color::white ? winc : binc;

  if (own_time > 0) {
    const auto budget = own_time / 30 + own_inc / 2;
    limits.max_time = std::chrono::milliseconds{std::max(10, budget - 50)};

    return limits;
  }

  limits.max_depth = 6;

  return limits;
}

auto handle_go_command(const std::vector<std::string_view>& tokens, position& current_position, std::ostream& output) -> void {
  const auto limits = parse_go_limits(tokens, current_position.side_to_move());
  const auto result = search(current_position, limits);

  fmt::print(output, "info depth {} score cp {} nodes {} time {}\n", result.depth, result.score, result.nodes, result.elapsed.count());

  if (result.best_move == move{}) {
    fmt::print(output, "bestmove 0000\n");
  } else {
    fmt::print(output, "bestmove {}\n", result.best_move);
  }

  output.flush();
}

auto run_uci_loop(std::istream& input, std::ostream& output) -> std::int32_t {
  auto current_position = position::from_fen(startpos_fen).value();

  auto line = std::string{};

  while (std::getline(input, line)) {
    const auto tokens = split_tokens(line);

    if (tokens.empty()) {
      continue;
    }

    const auto command = tokens[0];

    if (command == "uci") {
      fmt::print(output, "id name {}\n", engine_name);
      fmt::print(output, "id author {}\n", engine_author);
      fmt::print(output, "uciok\n");
      output.flush();
    } else if (command == "isready") {
      fmt::print(output, "readyok\n");
      output.flush();
    } else if (command == "ucinewgame") {
      current_position = position::from_fen(startpos_fen).value();
    } else if (command == "position") {
      handle_position_command(tokens, current_position);
    } else if (command == "go") {
      handle_go_command(tokens, current_position, output);
    } else if (command == "quit") {
      return 0;
    }
  }

  return 0;
}

} // namespace parallax

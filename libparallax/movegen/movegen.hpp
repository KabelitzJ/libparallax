// SPDX-License-Identifier: MIT
#ifndef PARALLAX_MOVEGEN_HPP_
#define PARALLAX_MOVEGEN_HPP_

#include <vector>

#include <libparallax/core/move.hpp>
#include <libparallax/core/position.hpp>

namespace parallax {

auto generate_pseudo_legal_moves(const position& current_position) -> std::vector<move>;

auto generate_legal_moves(const position& current_position) -> std::vector<move>;

} // namespace parallax

#endif // PARALLAX_MOVEGEN_HPP_
// SPDX-License-Identifier: MIT
#ifndef PARALLAX_EVAL_HPP_
#define PARALLAX_EVAL_HPP_

#include <libparallax/core/position.hpp>

namespace parallax {

auto evaluate(const position& current_position) noexcept -> int;

} // namespace parallax

#endif // PARALLAX_EVAL_HPP_
// SPDX-License-Identifier: MIT
#ifndef PARALLAX_UCI_HPP_
#define PARALLAX_UCI_HPP_

#include <cinttypes>

#include <iosfwd>

namespace parallax {

auto run_uci_loop(std::istream& input, std::ostream& output) -> std::int32_t;

} // namespace parallax

#endif // PARALLAX_UCI_HPP_
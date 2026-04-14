// SPDX-License-Identifier: MIT
#ifndef PARALLAX_UCI_HPP_
#define PARALLAX_UCI_HPP_

#include <iosfwd>

namespace parallax {

auto run_uci_loop(std::istream& input, std::ostream& output) -> int;

} // namespace parallax

#endif // PARALLAX_UCI_HPP_
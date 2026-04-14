// SPDX-License-Identifier: MIT
#include <iostream>

#include <libparallax/uci/uci.hpp>

auto main() -> int {
  return parallax::run_uci_loop(std::cin, std::cout);
}

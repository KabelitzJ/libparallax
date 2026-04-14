// SPDX-License-Identifier: MIT
#ifndef LIBPARALLAX_SEARCH_TRANSPOSITION_TABLE_HPP_
#define LIBPARALLAX_SEARCH_TRANSPOSITION_TABLE_HPP_

#include <cstdint>

#include <vector>

#include <libparallax/core/move.hpp>

namespace parallax {

enum class tt_bound : std::uint8_t {
  none,
  exact,
  lower,
  upper,
}; // enum class tt_bound

struct tt_entry {
  std::uint64_t key;
  move best_move;
  std::int32_t score;
  std::int16_t depth;
  tt_bound bound;
}; // struct tt_entry

class transposition_table {

public:

  explicit transposition_table(const std::size_t size_mb);

  auto clear() noexcept -> void;

  auto probe(const std::uint64_t key) const noexcept -> const tt_entry*;

  auto store(const std::uint64_t key, const move best_move, const std::int32_t score, const std::int16_t depth, const tt_bound bound) noexcept -> void;

private:

  auto index_for(const std::uint64_t key) const noexcept -> std::size_t {
    return static_cast<std::size_t>(key) & _mask;
  }

  std::vector<tt_entry> _entries;
  std::size_t _mask;

}; // class transposition_table

} // namespace parallax

#endif // LIBPARALLAX_SEARCH_TRANSPOSITION_TABLE_HPP_

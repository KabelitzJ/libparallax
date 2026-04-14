// SPDX-License-Identifier: MIT
#include <libparallax/search/transposition_table.hpp>

#include <bit>

namespace parallax {

transposition_table::transposition_table(const std::size_t size_mb) {
  const auto total_bytes = size_mb * 1024uz * 1024uz;
  const auto raw_count = total_bytes / sizeof(tt_entry);
  const auto power_of_two_count = std::bit_floor(raw_count);

  _entries.assign(power_of_two_count, tt_entry{});
  _mask = power_of_two_count - 1;
}

auto transposition_table::clear() noexcept -> void {
  for (auto& entry : _entries) {
    entry = tt_entry{};
  }
}

auto transposition_table::probe(const std::uint64_t key) const noexcept -> const tt_entry* {
  const auto& entry = _entries[index_for(key)];

  if (entry.key == key && entry.bound != tt_bound::none) {
    return &entry;
  }

  return nullptr;
}

auto transposition_table::store(const std::uint64_t key, const move best_move, const std::int32_t score, const std::int16_t depth, const tt_bound bound) noexcept -> void {
  auto& entry = _entries[index_for(key)];

  entry.key = key;
  entry.best_move = best_move;
  entry.score = score;
  entry.depth = depth;
  entry.bound = bound;
}

} // namespace parallax

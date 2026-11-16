#pragma once

#include <cstdint>

namespace sorting {

struct SortStats {
  std::uint64_t comparisons = 0;
  std::uint64_t swaps = 0;
  std::uint64_t writes = 0;
};

}  // namespace sorting

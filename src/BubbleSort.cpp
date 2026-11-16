#include "detail.hpp"

namespace sorting {
namespace detail {

// Optimized bubble sort with early-exit when a full pass makes no swaps.
SortStats bubble_sort(std::vector<int>& data, EventSink* sink) {
  SortStats stats{};
  const int n = static_cast<int>(data.size());
  if (n < 2) {
    return stats;
  }

  for (int end = n - 1; end > 0; --end) {
    bool swapped = false;
    for (int i = 0; i < end; ++i) {
      compare(sink, stats, i, i + 1);
      if (data[static_cast<std::size_t>(i)] > data[static_cast<std::size_t>(i + 1)]) {
        swap_at(sink, stats, data, i, i + 1);
        swapped = true;
      }
    }
    if (!swapped) {
      break;
    }
  }
  return stats;
}

}  // namespace detail
}  // namespace sorting

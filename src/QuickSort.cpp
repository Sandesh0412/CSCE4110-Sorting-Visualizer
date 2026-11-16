#include "detail.hpp"

namespace sorting {
namespace detail {
namespace {

// Lomuto partition with last element as pivot (required strategy).
int lomuto_partition(std::vector<int>& data, int lo, int hi, EventSink* sink, SortStats& stats) {
  emit(sink, SortEvent{EventType::Partition, lo, hi, 0});

  const int pivot_index = hi;
  const int pivot = data[static_cast<std::size_t>(pivot_index)];
  emit(sink, SortEvent{EventType::Partition, pivot_index, pivot_index, pivot});

  int store = lo;
  for (int i = lo; i < hi; ++i) {
    compare(sink, stats, i, pivot_index);
    if (data[static_cast<std::size_t>(i)] < pivot) {
      swap_at(sink, stats, data, store, i);
      ++store;
    }
  }
  swap_at(sink, stats, data, store, hi);
  return store;
}

// Recurse on the smaller side and iterate on the larger side so stack depth stays
// O(log n) even when last-element pivot yields unbalanced partitions (sorted /
// reverse-sorted / many duplicates). Pivot strategy is unchanged.
void quick_sort_rec(std::vector<int>& data, int lo, int hi, EventSink* sink, SortStats& stats) {
  while (lo < hi) {
    const int p = lomuto_partition(data, lo, hi, sink, stats);
    if (p - lo < hi - p) {
      if (lo < p - 1) {
        quick_sort_rec(data, lo, p - 1, sink, stats);
      }
      lo = p + 1;
    } else {
      if (p + 1 < hi) {
        quick_sort_rec(data, p + 1, hi, sink, stats);
      }
      hi = p - 1;
    }
  }
}

}  // namespace

SortStats quick_sort(std::vector<int>& data, EventSink* sink) {
  SortStats stats{};
  const int n = static_cast<int>(data.size());
  if (n < 2) {
    return stats;
  }
  quick_sort_rec(data, 0, n - 1, sink, stats);
  return stats;
}

}  // namespace detail
}  // namespace sorting

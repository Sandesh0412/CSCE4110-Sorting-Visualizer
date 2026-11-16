#pragma once

#include "sorting/SortEvent.hpp"
#include "sorting/SortStats.hpp"
#include "sorting/SortingEngine.hpp"

#include <vector>

namespace sorting {
namespace detail {

inline void emit(EventSink* sink, const SortEvent& e) {
  if (sink != nullptr && *sink) {
    (*sink)(e);
  }
}

inline void compare(EventSink* sink, SortStats& stats, int i, int j) {
  ++stats.comparisons;
  emit(sink, SortEvent{EventType::Compare, i, j, 0});
}

/// Swap primary[i] and primary[j]. Self-swaps (i == j) are no-ops: not counted, not emitted.
/// Mutation order: array is mutated, then the Swap event is emitted (listener sees post-swap).
inline void swap_at(EventSink* sink, SortStats& stats, std::vector<int>& data, int i, int j) {
  if (i == j) {
    return;
  }
  using std::swap;
  swap(data[static_cast<std::size_t>(i)], data[static_cast<std::size_t>(j)]);
  ++stats.swaps;
  emit(sink, SortEvent{EventType::Swap, i, j, 0});
}

/// Write to the primary array. Mutation happens before the Write event is emitted.
inline void write_at(EventSink* sink, SortStats& stats, std::vector<int>& data, int index, int value) {
  data[static_cast<std::size_t>(index)] = value;
  ++stats.writes;
  emit(sink, SortEvent{EventType::Write, index, kWritePrimary, value});
}

/// Count/emit a write into Merge Sort's temporary buffer (does not mutate primary).
inline void write_temp(EventSink* sink, SortStats& stats, int index, int value) {
  ++stats.writes;
  emit(sink, SortEvent{EventType::Write, index, kWriteTemp, value});
}

SortStats bubble_sort(std::vector<int>& data, EventSink* sink);
SortStats merge_sort(std::vector<int>& data, EventSink* sink);
SortStats quick_sort(std::vector<int>& data, EventSink* sink);

}  // namespace detail
}  // namespace sorting

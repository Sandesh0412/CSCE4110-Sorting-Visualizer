#pragma once

namespace sorting {

enum class EventType { Compare, Swap, Write, Partition };

/// Destination for Write events (stored in SortEvent::j).
constexpr int kWritePrimary = 0;  // primary std::vector<int> being sorted
constexpr int kWriteTemp = 1;     // Merge Sort temporary buffer

/// Operation event for visualizers / tracers.
///
/// Compare:   `i` and `j` are indices in the primary array. Non-mutating.
/// Swap:      primary[i] <-> primary[j]. Engine mutates the array, then emits.
///            Listener observes post-swap state. Self-swaps are not emitted.
/// Write:     `i` = index, `value` = written value, `j` = kWritePrimary or kWriteTemp.
///            Primary writes mutate then emit; temp writes do not touch primary.
/// Partition: Quicksort only.
///            - range: `i` = lo, `j` = hi (`i` != `j`), `value` unused
///            - pivot: `i` == `j` == pivot index, `value` = pivot value
struct SortEvent {
  EventType type;
  int i;
  int j;
  int value = 0;
};

}  // namespace sorting

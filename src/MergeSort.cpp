#include "detail.hpp"

#include <vector>

namespace sorting {
namespace detail {
namespace {

void merge_range(std::vector<int>& data, std::vector<int>& buf, int lo, int mid, int hi,
                 EventSink* sink, SortStats& stats) {
  int i = lo;
  int j = mid + 1;
  int k = lo;

  while (i <= mid && j <= hi) {
    // Comparisons use primary-array indices (values still live in `data` until copy-back).
    compare(sink, stats, i, j);
    if (data[static_cast<std::size_t>(i)] <= data[static_cast<std::size_t>(j)]) {
      buf[static_cast<std::size_t>(k)] = data[static_cast<std::size_t>(i)];
      write_temp(sink, stats, k, buf[static_cast<std::size_t>(k)]);
      ++i;
    } else {
      buf[static_cast<std::size_t>(k)] = data[static_cast<std::size_t>(j)];
      write_temp(sink, stats, k, buf[static_cast<std::size_t>(k)]);
      ++j;
    }
    ++k;
  }

  while (i <= mid) {
    buf[static_cast<std::size_t>(k)] = data[static_cast<std::size_t>(i)];
    write_temp(sink, stats, k, buf[static_cast<std::size_t>(k)]);
    ++i;
    ++k;
  }

  while (j <= hi) {
    buf[static_cast<std::size_t>(k)] = data[static_cast<std::size_t>(j)];
    write_temp(sink, stats, k, buf[static_cast<std::size_t>(k)]);
    ++j;
    ++k;
  }

  for (int t = lo; t <= hi; ++t) {
    write_at(sink, stats, data, t, buf[static_cast<std::size_t>(t)]);
  }
}

void merge_sort_rec(std::vector<int>& data, std::vector<int>& buf, int lo, int hi,
                    EventSink* sink, SortStats& stats) {
  if (lo >= hi) {
    return;
  }
  const int mid = lo + (hi - lo) / 2;
  merge_sort_rec(data, buf, lo, mid, sink, stats);
  merge_sort_rec(data, buf, mid + 1, hi, sink, stats);
  merge_range(data, buf, lo, mid, hi, sink, stats);
}

}  // namespace

SortStats merge_sort(std::vector<int>& data, EventSink* sink) {
  SortStats stats{};
  const int n = static_cast<int>(data.size());
  if (n < 2) {
    return stats;
  }
  std::vector<int> buf(static_cast<std::size_t>(n));
  merge_sort_rec(data, buf, 0, n - 1, sink, stats);
  return stats;
}

}  // namespace detail
}  // namespace sorting

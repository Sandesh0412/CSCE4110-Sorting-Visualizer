#pragma once

#include "sorting/Algorithm.hpp"
#include "sorting/SortEvent.hpp"
#include "sorting/SortStats.hpp"

#include <functional>
#include <vector>

namespace sorting {

using EventSink = std::function<void(const SortEvent&)>;

/// Sort `data` in place with the chosen algorithm. Returns fresh per-run SortStats
/// (counters always start at zero for each call).
///
/// @param sink Optional. nullptr (default) disables event delivery. Counters and
///        sort results are identical with or without a sink. The engine does not
///        retain an internal event history.
/// @note  Sink lifetime: `*sink` must remain valid for the duration of this call.
/// @note  Exceptions: if the sink throws, sorting stops immediately; `data` may be
///        partially sorted and the returned stats are not produced (exception propagates).
SortStats sort(Algorithm algo, std::vector<int>& data, EventSink* sink = nullptr);

}  // namespace sorting

#include "sorting/SortingEngine.hpp"

#include "detail.hpp"

#include <stdexcept>

namespace sorting {

SortStats sort(Algorithm algo, std::vector<int>& data, EventSink* sink) {
  switch (algo) {
    case Algorithm::Bubble:
      return detail::bubble_sort(data, sink);
    case Algorithm::Merge:
      return detail::merge_sort(data, sink);
    case Algorithm::Quick:
      return detail::quick_sort(data, sink);
  }
  throw std::invalid_argument("sorting::sort: unknown Algorithm");
}

}  // namespace sorting

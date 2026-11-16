#include "sorting/SortingEngine.hpp"

#include <iostream>
#include <vector>

namespace {

void print_stats(const char* name, const sorting::SortStats& s) {
  std::cout << name << ": comparisons=" << s.comparisons
            << " swaps=" << s.swaps
            << " writes=" << s.writes << '\n';
}

void run(sorting::Algorithm algo, const char* name, std::vector<int> data) {
  auto stats = sorting::sort(algo, data, nullptr);
  std::cout << name << " result: [";
  for (std::size_t i = 0; i < data.size(); ++i) {
    if (i) {
      std::cout << ", ";
    }
    std::cout << data[i];
  }
  std::cout << "]\n";
  print_stats(name, stats);
}

}  // namespace

int main() {
  const std::vector<int> sample{9, 3, 7, 1, 8, 2, 5, 4, 6, 0};
  run(sorting::Algorithm::Bubble, "Bubble", sample);
  run(sorting::Algorithm::Merge, "Merge", sample);
  run(sorting::Algorithm::Quick, "Quick", sample);
  return 0;
}

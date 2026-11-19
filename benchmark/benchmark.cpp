#include "benchmark.hpp"

#include "sorting/SortingEngine.hpp"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <random>
#include <stdexcept>
#include <vector>

namespace bench {

const char* algorithmName(sorting::Algorithm algo) {
  switch (algo) {
    case sorting::Algorithm::Bubble:
      return "Bubble";
    case sorting::Algorithm::Merge:
      return "Merge";
    case sorting::Algorithm::Quick:
      return "Quick";
  }
  return "Unknown";
}

const char* arrangementName(Arrangement arrangement) {
  switch (arrangement) {
    case Arrangement::Random:
      return "random";
    case Arrangement::Sorted:
      return "sorted";
    case Arrangement::Reverse:
      return "reverse";
    case Arrangement::NearlySorted:
      return "nearly_sorted";
    case Arrangement::Duplicates:
      return "duplicates";
  }
  return "unknown";
}

std::vector<int> makeArray(Arrangement arrangement, std::size_t size, std::uint32_t seed) {
  std::vector<int> data(size);
  std::mt19937 gen(seed + static_cast<std::uint32_t>(size) * 17u +
                   static_cast<std::uint32_t>(arrangement) * 101u);

  switch (arrangement) {
    case Arrangement::Random: {
      std::uniform_int_distribution<int> dist(0, static_cast<int>(size * 10 + 1));
      for (std::size_t i = 0; i < size; ++i) {
        data[i] = dist(gen);
      }
      break;
    }
    case Arrangement::Sorted: {
      for (std::size_t i = 0; i < size; ++i) {
        data[i] = static_cast<int>(i);
      }
      break;
    }
    case Arrangement::Reverse: {
      for (std::size_t i = 0; i < size; ++i) {
        data[i] = static_cast<int>(size - 1 - i);
      }
      break;
    }
    case Arrangement::NearlySorted: {
      for (std::size_t i = 0; i < size; ++i) {
        data[i] = static_cast<int>(i);
      }
      if (size >= 2) {
        const std::size_t swaps = std::max<std::size_t>(1, size / 20);
        std::uniform_int_distribution<std::size_t> dist(0, size - 1);
        for (std::size_t s = 0; s < swaps; ++s) {
          const std::size_t a = dist(gen);
          const std::size_t b = dist(gen);
          std::swap(data[a], data[b]);
        }
      }
      break;
    }
    case Arrangement::Duplicates: {
      // Few distinct keys — stresses last-element Quick pivot
      const int distinct = std::max(1, static_cast<int>(size / 10));
      std::uniform_int_distribution<int> dist(0, distinct);
      for (std::size_t i = 0; i < size; ++i) {
        data[i] = dist(gen);
      }
      break;
    }
  }

  return data;
}

RunResult measure(
    sorting::Algorithm algo,
    Arrangement arrangement,
    std::size_t size,
    std::uint32_t seed,
    int trials) {
  if (trials < 1) {
    throw std::invalid_argument("trials must be >= 1");
  }

  const auto prototype = makeArray(arrangement, size, seed);
  std::vector<double> durations;
  durations.reserve(static_cast<std::size_t>(trials));

  sorting::SortStats lastStats{};

  // Warmup
  {
    auto warm = prototype;
    sorting::sort(algo, warm, nullptr);
  }

  for (int t = 0; t < trials; ++t) {
    auto data = prototype;
    const auto start = std::chrono::steady_clock::now();
    lastStats = sorting::sort(algo, data, nullptr);
    const auto stop = std::chrono::steady_clock::now();
    durations.push_back(
        std::chrono::duration<double, std::milli>(stop - start).count());
  }

  std::sort(durations.begin(), durations.end());
  const double median = durations[static_cast<std::size_t>(trials) / 2];

  RunResult result;
  result.algorithm = algo;
  result.arrangement = arrangement;
  result.size = size;
  result.timeMs = median;
  result.stats = lastStats;
  return result;
}

std::vector<RunResult> runSuite(
    const std::vector<std::size_t>& sizes,
    const std::vector<Arrangement>& arrangements,
    const std::vector<sorting::Algorithm>& algorithms,
    std::uint32_t seed,
    int trials) {
  std::vector<RunResult> rows;
  rows.reserve(sizes.size() * arrangements.size() * algorithms.size());

  for (sorting::Algorithm algo : algorithms) {
    for (Arrangement arrangement : arrangements) {
      for (std::size_t size : sizes) {
        // Bubble at large n is intentionally included but slow; keep sizes modest.
        rows.push_back(measure(algo, arrangement, size, seed, trials));
      }
    }
  }
  return rows;
}

void writeCsv(const std::string& path, const std::vector<RunResult>& rows) {
  std::ofstream out(path);
  if (!out) {
    throw std::runtime_error("failed to open CSV for writing: " + path);
  }

  out << "algorithm,arrangement,size,time_ms,comparisons,swaps,writes\n";
  out << std::fixed;
  for (const auto& row : rows) {
    out << algorithmName(row.algorithm) << ','
        << arrangementName(row.arrangement) << ','
        << row.size << ','
        << row.timeMs << ','
        << row.stats.comparisons << ','
        << row.stats.swaps << ','
        << row.stats.writes << '\n';
  }
}

}  // namespace bench

#include "benchmark.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace {

void printUsage(const char* argv0) {
  std::cerr
      << "Usage: " << argv0
      << " [--out PATH] [--seed N] [--trials N] [--quick]\n"
      << "\n"
      << "Standalone sorting benchmark (no SFML).\n"
      << "Measures median wall time with events disabled, plus\n"
      << "comparisons / swaps / writes from SortStats.\n"
      << "\n"
      << "Options:\n"
      << "  --out PATH   CSV output path (default: results/benchmark.csv)\n"
      << "  --seed N     RNG seed (default: 4110)\n"
      << "  --trials N   Timed trials per cell; median reported (default: 5)\n"
      << "  --quick      Smaller size set for a fast smoke run\n"
      << "  -h, --help   Show this help\n";
}

}  // namespace

int main(int argc, char** argv) {
  std::string outPath = "results/benchmark.csv";
  std::uint32_t seed = 4110;
  int trials = 5;
  bool quick = false;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    auto need = [&](const char* name) -> std::string {
      if (i + 1 >= argc) {
        std::cerr << "Missing value for " << name << '\n';
        std::exit(2);
      }
      return argv[++i];
    };

    if (arg == "-h" || arg == "--help") {
      printUsage(argv[0]);
      return 0;
    }
    if (arg == "--out") {
      outPath = need("--out");
    } else if (arg == "--seed") {
      seed = static_cast<std::uint32_t>(std::stoul(need("--seed")));
    } else if (arg == "--trials") {
      trials = std::stoi(need("--trials"));
    } else if (arg == "--quick") {
      quick = true;
    } else {
      std::cerr << "Unknown argument: " << arg << '\n';
      printUsage(argv[0]);
      return 2;
    }
  }

  const std::vector<std::size_t> sizes = quick
      ? std::vector<std::size_t>{64, 128, 256}
      : std::vector<std::size_t>{64, 128, 256, 512, 1024};

  const std::vector<bench::Arrangement> arrangements = {
      bench::Arrangement::Random,
      bench::Arrangement::Sorted,
      bench::Arrangement::Reverse,
      bench::Arrangement::NearlySorted,
      bench::Arrangement::Duplicates,
  };

  const std::vector<sorting::Algorithm> algorithms = {
      sorting::Algorithm::Bubble,
      sorting::Algorithm::Merge,
      sorting::Algorithm::Quick,
  };

  std::cout << "Running benchmark suite"
            << " (sizes=" << sizes.size()
            << ", arrangements=" << arrangements.size()
            << ", algorithms=" << algorithms.size()
            << ", trials=" << trials
            << ", seed=" << seed << ")\n";

  const auto rows =
      bench::runSuite(sizes, arrangements, algorithms, seed, trials);

  const auto parent = std::filesystem::path(outPath).parent_path();
  if (!parent.empty()) {
    std::filesystem::create_directories(parent);
  }

  bench::writeCsv(outPath, rows);

  std::cout << "Wrote " << rows.size() << " rows to " << outPath << '\n';
  std::cout << "algorithm,arrangement,size,time_ms,comparisons,swaps,writes\n";
  for (const auto& row : rows) {
    std::cout << bench::algorithmName(row.algorithm) << ','
              << bench::arrangementName(row.arrangement) << ','
              << row.size << ','
              << row.timeMs << ','
              << row.stats.comparisons << ','
              << row.stats.swaps << ','
              << row.stats.writes << '\n';
  }

  return 0;
}

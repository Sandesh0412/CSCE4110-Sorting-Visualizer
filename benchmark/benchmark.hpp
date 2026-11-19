#pragma once

#include "sorting/Algorithm.hpp"
#include "sorting/SortStats.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace bench {

enum class Arrangement {
    Random,
    Sorted,
    Reverse,
    NearlySorted,
    Duplicates
};

struct RunResult {
    sorting::Algorithm algorithm;
    Arrangement arrangement;
    std::size_t size = 0;
    double timeMs = 0.0;
    sorting::SortStats stats;
};

const char* algorithmName(sorting::Algorithm algo);
const char* arrangementName(Arrangement arrangement);

std::vector<int> makeArray(Arrangement arrangement, std::size_t size, std::uint32_t seed);

/// Median wall time over `trials` runs (events disabled). Stats from the last run.
RunResult measure(
    sorting::Algorithm algo,
    Arrangement arrangement,
    std::size_t size,
    std::uint32_t seed,
    int trials
);

std::vector<RunResult> runSuite(
    const std::vector<std::size_t>& sizes,
    const std::vector<Arrangement>& arrangements,
    const std::vector<sorting::Algorithm>& algorithms,
    std::uint32_t seed,
    int trials
);

void writeCsv(const std::string& path, const std::vector<RunResult>& rows);

}  // namespace bench

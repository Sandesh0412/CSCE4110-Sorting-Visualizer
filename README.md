# CSCE4110 Sorting Visualizer — Sorting Engine

C++17 sorting-engine library for the team Sorting Algorithm Visualizer & Benchmark project.

**Current status (Sandesh / `sandeshpariyar`):** the sorting algorithms, operation events, and per-run counters are implemented and tested. SFML visualization, playback controls, benchmark graphs, PDF report, and video are **not** in this branch — those belong to teammates.

| Person | Focus | Status here |
|--------|--------|-------------|
| **Pariyar Sandesh** | Sorting algorithms & engine logic | Implemented in this branch |
| **Benjamin** | SFML visualizer consuming events | Integration pending |
| **Fidel** | Benchmark UI / timing | Integration pending |

This library has **no SFML dependency**.

## Prerequisites

- C++17 compiler (verified with **g++ 13.3**)
- **CMake ≥ 3.16**
- On this environment the default `c++` may be clang without a usable `-lstdc++`; prefer `CXX=g++`

## Build and test (verified)

Out-of-tree builds only:

```bash
# Debug
CXX=g++ cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug
ctest --test-dir build-debug --output-on-failure

# Release
CXX=g++ cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
ctest --test-dir build-release --output-on-failure
```

Optional sanitizers (when the toolchain supports them):

```bash
CXX=g++ cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"
cmake --build build-asan
ctest --test-dir build-asan --output-on-failure
```

Targets: `sorting_engine` (static lib), `sorting_tests`, `sorting_demo`.

## Directory layout

```
.
├── CMakeLists.txt
├── README.md
├── include/sorting/
│   ├── Algorithm.hpp
│   ├── SortEvent.hpp
│   ├── SortStats.hpp
│   └── SortingEngine.hpp
├── src/
│   ├── BubbleSort.cpp
│   ├── MergeSort.cpp
│   ├── QuickSort.cpp
│   ├── SortingEngine.cpp
│   └── detail.hpp          # internal helpers (not public API)
├── tests/test_sorting.cpp
└── demo/main.cpp           # tiny CLI: sort a sample and print stats
```

## Minimal API example

```cpp
#include "sorting/SortingEngine.hpp"
#include <vector>

int main() {
  std::vector<int> data{5, 1, 4, 2, 3};

  sorting::EventSink sink = [](const sorting::SortEvent& e) {
    // Benjamin: drive highlights / bar swaps from e
  };

  sorting::SortStats stats =
      sorting::sort(sorting::Algorithm::Merge, data, &sink);

  // Fidel: pass nullptr (or omit) to skip events; stats still fill in
  auto timed = sorting::sort(sorting::Algorithm::Quick, data, nullptr);
  (void)stats;
  (void)timed;
}
```

## Algorithms

| Algorithm | Behavior | Average | Worst | Stable? |
|-----------|----------|---------|-------|---------|
| **Bubble** | Adjacent compare/swap; **early-exit** after a swap-free pass | O(n²) | O(n²) | Yes |
| **Merge** | Recursive split + merge into a temp buffer, then copy back | O(n log n) | O(n log n) | Yes |
| **Quick** | **Last-element pivot**, **Lomuto** partition; recurse smaller side / iterate larger (stack-safe; pivot strategy unchanged) | O(n log n) | **O(n²)** on sorted / reverse / many equal keys with this pivot | No |

Empty and single-element inputs are no-ops with all counters zero.

**Quicksort limitation:** last-element pivot remains intentionally fixed for the assignment. Sorted, reversed, and duplicate-heavy inputs are quadratic in comparisons/swaps. Prefer Merge (or a different pivot policy later) for large adversarial inputs in benchmarks.

## Counters (`SortStats`)

Returned by every `sort()` call. Always start at **zero for that run** (no carry-over).

| Field | Counts |
|-------|--------|
| `comparisons` | Each value comparison performed by the algorithm |
| `swaps` | Each actual exchange of two **distinct** primary indices (`i != j`). Self-swaps are skipped and **not** counted |
| `writes` | Each assignment into the Merge temp buffer **or** back into the primary array. Bubble/Quick do not use writes |

Disabling the event sink does **not** change counter values or sort results. Instrumentation (incrementing counters, optional callbacks) still costs CPU — Fidel should treat engine timings as instrumented unless measuring a stripped build.

## Event contract (Benjamin)

```cpp
enum class EventType { Compare, Swap, Write, Partition };

struct SortEvent {
  EventType type;
  int i;
  int j;
  int value = 0;
};

constexpr int kWritePrimary = 0;
constexpr int kWriteTemp = 1;
```

| Type | Meaning | `i` | `j` | `value` | Mutates primary? |
|------|---------|-----|-----|---------|------------------|
| `Compare` | Compare primary[i] vs primary[j] | index | index | unused | No |
| `Swap` | Swap primary[i] and primary[j] | index | index | unused | **Yes** (mutated **before** callback) |
| `Write` | Assign `value` | index | `kWritePrimary` or `kWriteTemp` | written value | Only if `j == kWritePrimary` (mutated before callback) |
| `Partition` range | Quicksort subarray | lo | hi | unused | No |
| `Partition` pivot | Quicksort pivot highlight | pivot | pivot (`i == j`) | pivot value | No |

### Merge Sort specifics

- Comparisons during merge use **primary-array indices**; values still live in `data` until copy-back.
- Filling the temp buffer emits `Write` with `j == kWriteTemp` (does not change primary).
- Copy-back emits `Write` with `j == kWritePrimary`.

### Replay guidance

From the initial array, apply only:

1. every `Swap`, and
2. every `Write` with `j == kWritePrimary`

Ignore `Compare`, `Partition`, and temp `Write`s for reconstructing the primary array. After the run, the replayed array must match the engine result (covered by tests).

### Sink rules

- Type: `std::function<void(const SortEvent&)>`; pass `EventSink*` or `nullptr`.
- No sink ⇒ no event storage inside the engine.
- Sink must outlive the `sort()` call.
- If the sink throws, `sort` aborts; `data` may be partially sorted.

## Teammate integration notes

**Benjamin:** link `sorting_engine`, call `sort` with a sink, map `Compare`/`Swap`/`Write`/`Partition` to SFML highlights. Prefer primary mutations for bar updates; temp writes can drive a secondary buffer visualization if desired.

**Fidel:** call `sort(algo, data, nullptr)` for counters without event overhead. Still expect instrumentation cost from counter updates. Do not assume wall-clock times equal an uninstrumented textbook implementation.

## Known limitations

- Quicksort last-element pivot ⇒ quadratic worst case (documented above).
- Engine sorts `std::vector<int>` only.
- No SFML, UI, charts, PDF, or video in this contribution.
- Algorithms never call `std::sort` (tests use it only as an oracle).

## Tests actually covered

Custom runner in `tests/test_sorting.cpp` (assertions stay active in Release — no reliance on `assert`):

- Empty, single, two-element
- Duplicates / all-equal
- Sorted and reverse-sorted
- Negatives, `INT_MIN` / `INT_MAX`
- Reproducible random sizes vs `std::sort`
- Every algorithm with and without a sink (same result + counters)
- Counter independence across runs
- Exact counters on small hand-checked Bubble/Merge/Quick examples
- Event index/value validity
- Mutation-event replay equals engine output
- Quicksort sorted / reversed / duplicate inputs at n = 64, 128, 256

**Verified on this branch:** Debug, Release, and ASan+UBSan builds each reported `Passed: 186  Failed: 0` via `ctest` / `sorting_tests` (g++ 13.3, CMake 3.28.3).

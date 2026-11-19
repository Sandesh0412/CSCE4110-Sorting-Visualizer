# CSCE 4110 Sorting Visualizer

C++17 sorting-engine library, SFML visualizer, and standalone benchmark suite for the team Sorting Algorithm Visualizer & Benchmark project.

| Person | Focus | Status |
|--------|--------|--------|
| **Pariyar Sandesh** | Sorting algorithms & engine logic | Implemented |
| **Benjamin** | SFML visualizer consuming events | Implemented under `visualizer/` |
| **Fidel** | Benchmark timing / graphs | Implemented under `benchmark/`, `scripts/`, `results/` |

PDF report, presentation, and demo video are separate deliverables (not covered here).

## Prerequisites

- **C++17** compiler (verified with **g++ 13.3**)
- **CMake ≥ 3.16**
- Prefer `CXX=g++` if the default `c++` is clang without a usable `-lstdc++`
- **Visualizer only:** Git + network (CMake FetchContent downloads SFML 3.0.2); Linux also needs X11/OpenGL build deps such as `libxrandr-dev`, `libxcursor-dev`, `libxi-dev`, `libudev-dev`, `libfreetype-dev`, `libgl1-mesa-dev`
- **Graphs:** Python 3 (stdlib only) to regenerate SVG charts from CSV

The sorting engine and benchmark have **no SFML dependency**.

## Build and test (engine)

Out-of-tree builds:

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

Targets always built: `sorting_engine` (static lib), `sorting_tests`, `sorting_demo`, and (by default) `sorting_benchmark`.

Optional sanitizers (when the toolchain supports them):

```bash
CXX=g++ cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"
cmake --build build-asan
ctest --test-dir build-asan --output-on-failure
```

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
│   └── detail.hpp
├── tests/test_sorting.cpp
├── demo/main.cpp
├── visualizer/
│   ├── main.cpp
│   ├── visualizer.cpp
│   └── visualizer.hpp
├── benchmark/
│   ├── main.cpp
│   ├── benchmark.cpp
│   └── benchmark.hpp
├── scripts/plot_benchmark.py
├── results/                 # sample CSV + SVG charts
└── assets/fonts/Inter-Regular.ttf
```

## Minimal API example

```cpp
#include "sorting/SortingEngine.hpp"
#include <vector>

int main() {
  std::vector<int> data{5, 1, 4, 2, 3};

  sorting::EventSink sink = [](const sorting::SortEvent& e) {
    // drive highlights / bar swaps from e
  };

  sorting::SortStats stats =
      sorting::sort(sorting::Algorithm::Merge, data, &sink);

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
| **Quick** | **Last-element pivot**, **Lomuto** partition; recurse smaller side / iterate larger | O(n log n) | **O(n²)** on sorted / reverse / many equal keys with this pivot | No |

Empty and single-element inputs are no-ops with all counters zero.

**Quicksort limitation:** last-element pivot remains intentionally fixed for the assignment. Sorted, reversed, and duplicate-heavy inputs are quadratic in comparisons/swaps.

## Counters (`SortStats`)

| Field | Counts |
|-------|--------|
| `comparisons` | Each value comparison performed by the algorithm |
| `swaps` | Each exchange of two **distinct** primary indices (`i != j`) |
| `writes` | Each assignment into the Merge temp buffer **or** back into the primary array. Bubble/Quick do not use writes |

Disabling the event sink does **not** change counter values or sort results.

## Event contract

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

| Type | Meaning | `i` | `j` | Mutates primary? |
|------|---------|-----|-----|------------------|
| `Compare` | Compare primary[i] vs primary[j] | index | index | No |
| `Swap` | Swap primary[i] and primary[j] | index | index | **Yes** (before callback) |
| `Write` | Assign `value` | index | `kWritePrimary` or `kWriteTemp` | Only if `j == kWritePrimary` |
| `Partition` | Quicksort range or pivot highlight | lo/pivot | hi/pivot | No |

### Replay guidance

From the initial array, apply only every `Swap` and every `Write` with `j == kWritePrimary`. Ignore `Compare`, `Partition`, and temp writes.

## Sorting Visualizer (SFML)

Optional CMake target (`BUILD_VISUALIZER`, default **OFF**). SFML 3.0.2 is fetched automatically.

### Build and run

```bash
# Linux / macOS (needs X11/OpenGL -dev packages on Linux)
CXX=g++ cmake -S . -B build-visualizer -DBUILD_VISUALIZER=ON
cmake --build build-visualizer --target sorting_visualizer
./build-visualizer/sorting_visualizer

# Windows (MinGW example)
cmake -S . -B build-visualizer -G "MinGW Makefiles" -DBUILD_VISUALIZER=ON
cmake --build build-visualizer --target sorting_visualizer -j 4
.\build-visualizer\sorting_visualizer.exe
```

Run from the repository root so `assets/fonts/Inter-Regular.ttf` resolves.

### Keyboard controls

| Key | Action |
|---|---|
| 1 / 2 / 3 | Bubble / Merge / Quick |
| Space | Play / Pause (replays when complete) |
| N or Right Arrow | **Single-step** one event |
| R | Reset animation |
| Up / Down | Faster / slower playback |
| G | Generate a **random** array (prompts for size 5–80) |
| I | Enter a **custom integer array** (spaces or commas) |
| Enter / Esc | Confirm / cancel text input |

On-screen stats (comparisons, swaps, writes, 31-run median time) appear when playback completes.

## Standalone benchmark

No SFML. Built by default (`BUILD_BENCHMARK=ON`).

Measures **median wall-clock time** (events disabled) plus `comparisons` / `swaps` / `writes` across:

- **Sizes:** 64, 128, 256, 512, 1024 (or `--quick`: 64, 128, 256)
- **Arrangements:** random, sorted, reverse, nearly_sorted, duplicates
- **Algorithms:** Bubble, Merge, Quick

```bash
CXX=g++ cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug --target sorting_benchmark

# Full suite → CSV
./build-debug/sorting_benchmark --out results/benchmark.csv --seed 4110 --trials 5

# Fast smoke
./build-debug/sorting_benchmark --quick --out /tmp/bench-quick.csv
```

### Regenerating graphs

Sample CSV and SVG charts are committed under `results/`. Regenerate with:

```bash
python3 scripts/plot_benchmark.py results/benchmark.csv --outdir results
```

Produces:

- `results/time_by_size_random.svg`
- `results/comparisons_by_size_random.svg`
- `results/time_by_arrangement_n1024.svg` (largest `n` in the CSV)

Open the SVGs in any browser or image viewer.

## Tests covered

Custom runner in `tests/test_sorting.cpp` (assertions stay active in Release):

- Empty, single, two-element; duplicates / all-equal; sorted and reverse-sorted
- Negatives, `INT_MIN` / `INT_MAX`; reproducible random vs `std::sort`
- Every algorithm with and without a sink; counter independence
- Exact counters on small hand-checked examples; event validity; mutation replay
- Quicksort adversarial sizes at n = 64, 128, 256

**Verified:** Debug build `Passed: 186  Failed: 0` via `ctest` / `sorting_tests`.

## Known limitations

- Quicksort last-element pivot ⇒ quadratic worst case (documented above).
- Engine sorts `std::vector<int>` only.
- Algorithms never call `std::sort` (tests use it only as an oracle).
- Visualizer timing panel is a small in-UI median; prefer `sorting_benchmark` for published graphs.
- SFML visualizer needs a graphical display (and the Linux X11/OpenGL packages listed above).

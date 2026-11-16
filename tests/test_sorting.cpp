#include "sorting/SortingEngine.hpp"

#include <algorithm>
#include <climits>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace {

int g_failed = 0;
int g_passed = 0;

void check(bool cond, const std::string& msg) {
  if (cond) {
    ++g_passed;
  } else {
    ++g_failed;
    std::cerr << "FAIL: " << msg << '\n';
  }
}

const char* algo_name(sorting::Algorithm a) {
  switch (a) {
    case sorting::Algorithm::Bubble:
      return "Bubble";
    case sorting::Algorithm::Merge:
      return "Merge";
    case sorting::Algorithm::Quick:
      return "Quick";
  }
  return "?";
}

bool indices_ok(const sorting::SortEvent& e, int n) {
  if (n <= 0) {
    return false;
  }
  switch (e.type) {
    case sorting::EventType::Compare:
    case sorting::EventType::Swap:
      return e.i >= 0 && e.j >= 0 && e.i < n && e.j < n;
    case sorting::EventType::Write:
      return e.i >= 0 && e.i < n &&
             (e.j == sorting::kWritePrimary || e.j == sorting::kWriteTemp);
    case sorting::EventType::Partition:
      return e.i >= 0 && e.j >= 0 && e.i < n && e.j < n && e.i <= e.j;
  }
  return false;
}

void replay_mutations(std::vector<int>& mirror, const sorting::SortEvent& e) {
  if (e.type == sorting::EventType::Swap) {
    using std::swap;
    swap(mirror[static_cast<std::size_t>(e.i)], mirror[static_cast<std::size_t>(e.j)]);
  } else if (e.type == sorting::EventType::Write && e.j == sorting::kWritePrimary) {
    mirror[static_cast<std::size_t>(e.i)] = e.value;
  }
}

void expect_sorted_vs_std(sorting::Algorithm algo, std::vector<int> input, const std::string& label,
                          bool expect_comparisons) {
  std::vector<int> expected = input;
  std::sort(expected.begin(), expected.end());

  auto stats = sorting::sort(algo, input, nullptr);
  check(input == expected, std::string(algo_name(algo)) + " sorts correctly: " + label);

  if (expect_comparisons) {
    check(stats.comparisons > 0,
          std::string(algo_name(algo)) + " comparisons > 0: " + label);
  } else {
    check(stats.comparisons == 0 && stats.swaps == 0 && stats.writes == 0,
          std::string(algo_name(algo)) + " zero work on trivial: " + label);
  }
}

void test_with_and_without_sink(sorting::Algorithm algo, std::vector<int> input,
                                const std::string& label) {
  std::vector<int> a = input;
  std::vector<int> b = input;
  auto stats_none = sorting::sort(algo, a, nullptr);

  std::size_t events = 0;
  sorting::EventSink sink = [&](const sorting::SortEvent& e) {
    ++events;
    check(indices_ok(e, static_cast<int>(input.size())),
          std::string(algo_name(algo)) + " event indices valid: " + label);
  };
  auto stats_sink = sorting::sort(algo, b, &sink);

  check(a == b, std::string(algo_name(algo)) + " same result with/without sink: " + label);
  check(stats_none.comparisons == stats_sink.comparisons &&
            stats_none.swaps == stats_sink.swaps && stats_none.writes == stats_sink.writes,
        std::string(algo_name(algo)) + " same counters with/without sink: " + label);
  if (input.size() >= 2) {
    check(events > 0, std::string(algo_name(algo)) + " emits events for n>=2: " + label);
  } else {
    check(events == 0, std::string(algo_name(algo)) + " no events for trivial: " + label);
  }
}

void test_replay(sorting::Algorithm algo, std::vector<int> input, const std::string& label) {
  std::vector<int> working = input;
  std::vector<int> mirror = input;
  sorting::EventSink sink = [&](const sorting::SortEvent& e) { replay_mutations(mirror, e); };
  sorting::sort(algo, working, &sink);
  check(mirror == working,
        std::string(algo_name(algo)) + " mutation replay matches result: " + label);
}

void test_counter_reset(sorting::Algorithm algo) {
  std::vector<int> first{5, 4, 3, 2, 1};
  auto s1 = sorting::sort(algo, first, nullptr);
  check(s1.comparisons > 0, std::string(algo_name(algo)) + " first run has work");

  std::vector<int> second{1, 2};
  auto s2 = sorting::sort(algo, second, nullptr);
  // Fresh SortStats per call — second run cannot include first-run totals.
  check(s2.comparisons < s1.comparisons,
        std::string(algo_name(algo)) + " second run counters independent of first");

  std::vector<int> third{};
  auto s3 = sorting::sort(algo, third, nullptr);
  check(s3.comparisons == 0 && s3.swaps == 0 && s3.writes == 0,
        std::string(algo_name(algo)) + " empty run resets to zero counters");
}

std::vector<int> random_vec(std::size_t n, unsigned seed) {
  std::mt19937 rng(seed);
  std::uniform_int_distribution<int> dist(-1000, 1000);
  std::vector<int> v(n);
  for (auto& x : v) {
    x = dist(rng);
  }
  return v;
}

void test_exact_bubble_two() {
  std::vector<int> data{2, 1};
  auto s = sorting::sort(sorting::Algorithm::Bubble, data, nullptr);
  check(data == std::vector<int>({1, 2}), "Bubble [2,1] result");
  check(s.comparisons == 1 && s.swaps == 1 && s.writes == 0, "Bubble [2,1] counters");
}

void test_exact_bubble_three() {
  // [3,1,2]: pass1 swap(0,1)->[1,3,2], swap(1,2)->[1,2,3]; pass2 compare(0,1) no swap, exit
  std::vector<int> data{3, 1, 2};
  auto s = sorting::sort(sorting::Algorithm::Bubble, data, nullptr);
  check(data == std::vector<int>({1, 2, 3}), "Bubble [3,1,2] result");
  check(s.comparisons == 3 && s.swaps == 2 && s.writes == 0, "Bubble [3,1,2] counters");
}

void test_exact_quick_two() {
  std::vector<int> data{2, 1};
  auto s = sorting::sort(sorting::Algorithm::Quick, data, nullptr);
  check(data == std::vector<int>({1, 2}), "Quick [2,1] result");
  check(s.comparisons == 1 && s.swaps == 1 && s.writes == 0, "Quick [2,1] counters");
}

void test_exact_merge_two() {
  std::vector<int> data{2, 1};
  auto s = sorting::sort(sorting::Algorithm::Merge, data, nullptr);
  check(data == std::vector<int>({1, 2}), "Merge [2,1] result");
  // 1 compare; 2 temp writes + 2 primary writes
  check(s.comparisons == 1 && s.swaps == 0 && s.writes == 4, "Merge [2,1] counters");
}

void test_merge_temp_vs_primary_events() {
  std::vector<int> data{2, 1};
  int temp_writes = 0;
  int primary_writes = 0;
  sorting::EventSink sink = [&](const sorting::SortEvent& e) {
    if (e.type == sorting::EventType::Write) {
      if (e.j == sorting::kWriteTemp) {
        ++temp_writes;
      } else if (e.j == sorting::kWritePrimary) {
        ++primary_writes;
      }
    }
  };
  sorting::sort(sorting::Algorithm::Merge, data, &sink);
  check(temp_writes == 2 && primary_writes == 2, "Merge distinguishes temp vs primary writes");
}

void test_self_swap_not_counted() {
  // Lomuto on sorted [1,2]: final pivot swap is store==hi (self-index) → not counted.
  // Equal values at distinct indices still count when swapped (e.g. all-equal arrays).
  std::vector<int> data{1, 2};
  auto s = sorting::sort(sorting::Algorithm::Quick, data, nullptr);
  check(s.comparisons == 1 && s.swaps == 0, "Quick [1,2]: self-index pivot swap omitted");
}

void run_suite_for(sorting::Algorithm algo) {
  expect_sorted_vs_std(algo, {}, "empty", false);
  expect_sorted_vs_std(algo, {42}, "single", false);
  expect_sorted_vs_std(algo, {2, 1}, "two elements", true);
  expect_sorted_vs_std(algo, {1, 1, 1, 1}, "all duplicates", true);
  expect_sorted_vs_std(algo, {1, 2, 3, 4, 5}, "already sorted", true);
  expect_sorted_vs_std(algo, {5, 4, 3, 2, 1}, "reverse sorted", true);
  expect_sorted_vs_std(algo, {7, 7, 3, 7, 1, 3}, "mixed duplicates", true);
  expect_sorted_vs_std(algo, {-3, -1, -2, 0, 2, -5}, "negatives", true);
  expect_sorted_vs_std(algo, {INT_MAX, INT_MIN, 0, INT_MAX, INT_MIN}, "int extremes", true);

  for (std::size_t n : {10u, 50u, 100u, 250u}) {
    expect_sorted_vs_std(algo, random_vec(n, static_cast<unsigned>(1000 + n)),
                         "random n=" + std::to_string(n), true);
  }

  test_with_and_without_sink(algo, {4, 1, 3, 2}, "small");
  test_with_and_without_sink(algo, {}, "empty");
  test_replay(algo, {9, 3, 7, 1, 8, 2, 5, 4, 6, 0}, "sample");
  test_replay(algo, {1, 1, 2, 2, 1}, "dups");
  test_counter_reset(algo);
}

void test_quick_adversarial_bounded() {
  // Practical sizes: last-element pivot is quadratic-time on sorted/reversed, but
  // smaller-side recursion keeps stack safe. Keep n modest for CI runtime.
  for (int n : {64, 128, 256}) {
    std::vector<int> sorted(static_cast<std::size_t>(n));
    std::vector<int> reversed(static_cast<std::size_t>(n));
    std::vector<int> dups(static_cast<std::size_t>(n), 7);
    for (int i = 0; i < n; ++i) {
      sorted[static_cast<std::size_t>(i)] = i;
      reversed[static_cast<std::size_t>(i)] = n - 1 - i;
    }
    expect_sorted_vs_std(sorting::Algorithm::Quick, sorted,
                         "quick sorted n=" + std::to_string(n), true);
    expect_sorted_vs_std(sorting::Algorithm::Quick, reversed,
                         "quick reversed n=" + std::to_string(n), true);
    expect_sorted_vs_std(sorting::Algorithm::Quick, dups,
                         "quick dups n=" + std::to_string(n), true);
    test_replay(sorting::Algorithm::Quick, sorted, "quick sorted replay n=" + std::to_string(n));
  }
}

}  // namespace

int main() {
  const sorting::Algorithm algos[] = {
      sorting::Algorithm::Bubble,
      sorting::Algorithm::Merge,
      sorting::Algorithm::Quick,
  };

  for (auto algo : algos) {
    run_suite_for(algo);
  }

  test_exact_bubble_two();
  test_exact_bubble_three();
  test_exact_quick_two();
  test_exact_merge_two();
  test_merge_temp_vs_primary_events();
  test_self_swap_not_counted();
  test_quick_adversarial_bounded();

  {
    std::vector<int> data{4, 1, 3, 2};
    auto m = sorting::sort(sorting::Algorithm::Merge, data, nullptr);
    check(m.writes > 0, "Merge records writes");
  }
  {
    std::vector<int> data{4, 1, 3, 2};
    auto q = sorting::sort(sorting::Algorithm::Quick, data, nullptr);
    check(q.swaps > 0, "Quick records swaps on unsorted input");
  }
  {
    std::vector<int> data{4, 1, 3, 2};
    auto b = sorting::sort(sorting::Algorithm::Bubble, data, nullptr);
    check(b.swaps > 0, "Bubble records swaps on unsorted input");
  }

  std::cout << "Passed: " << g_passed << "  Failed: " << g_failed << '\n';
  return g_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

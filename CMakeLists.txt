cmake_minimum_required(VERSION 3.16)
project(sorting_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(sorting_engine STATIC
  src/BubbleSort.cpp
  src/MergeSort.cpp
  src/QuickSort.cpp
  src/SortingEngine.cpp
)
target_include_directories(sorting_engine PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

enable_testing()

add_executable(sorting_tests tests/test_sorting.cpp)
target_link_libraries(sorting_tests PRIVATE sorting_engine)
add_test(NAME sorting_tests COMMAND sorting_tests)

add_executable(sorting_demo demo/main.cpp)
target_link_libraries(sorting_demo PRIVATE sorting_engine)

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

# SFML Sorting Visualizer (optional)
option(BUILD_VISUALIZER "Build the SFML sorting visualizer" OFF)

if(BUILD_VISUALIZER)
    include(FetchContent)

    FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.0.2
        GIT_SHALLOW TRUE
    )

    set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(SFML)

    add_executable(sorting_visualizer
      visualizer/main.cpp
      visualizer/visualizer.cpp
    )

    target_include_directories(sorting_visualizer PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/visualizer
    )

    target_link_libraries(sorting_visualizer PRIVATE
        sorting_engine
        SFML::Graphics
    )
endif()

# Standalone benchmark CLI (no SFML)
option(BUILD_BENCHMARK "Build the sorting benchmark CLI" ON)

if(BUILD_BENCHMARK)
    add_executable(sorting_benchmark
      benchmark/main.cpp
      benchmark/benchmark.cpp
    )
    target_include_directories(sorting_benchmark PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/benchmark
    )
    target_link_libraries(sorting_benchmark PRIVATE sorting_engine)
endif()

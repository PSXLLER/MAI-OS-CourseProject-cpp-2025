cmake_minimum_required(VERSION 3.16)
project(OS_CourseProject_Allocators)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_ROOT ${CMAKE_SOURCE_DIR}/cp_var19)
set(SRC_DIR      ${PROJECT_ROOT}/src)
set(INCLUDE_DIR  ${PROJECT_ROOT}/include)
set(TEST_DIR     ${PROJECT_ROOT}/tests)

include_directories(${INCLUDE_DIR})

set(MAIN_SOURCES
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/freelist_allocator.cpp
    ${SRC_DIR}/pow2_allocator.cpp
    ${SRC_DIR}/allocator_common.cpp
)

set(DEMO_SOURCES
    ${SRC_DIR}/demo_main.cpp
    ${SRC_DIR}/freelist_allocator.cpp
    ${SRC_DIR}/pow2_allocator.cpp
    ${SRC_DIR}/allocator_common.cpp
)

set(BENCHMARK_SOURCES
    ${SRC_DIR}/benchmark.cpp
    ${SRC_DIR}/freelist_allocator.cpp
    ${SRC_DIR}/pow2_allocator.cpp
    ${SRC_DIR}/allocator_common.cpp
)

set(TEST_SOURCES
    ${TEST_DIR}/gtest_allocators.cpp
    ${SRC_DIR}/freelist_allocator.cpp
    ${SRC_DIR}/pow2_allocator.cpp
    ${SRC_DIR}/allocator_common.cpp
)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(main ${MAIN_SOURCES})

add_executable(demo ${DEMO_SOURCES})

add_executable(benchmark ${BENCHMARK_SOURCES})

add_executable(allocator_tests ${TEST_SOURCES})
target_link_libraries(allocator_tests gtest gtest_main pthread)

include(GoogleTest)
gtest_discover_tests(allocator_tests)

message(STATUS "Source dir:     ${SRC_DIR}")
message(STATUS "Include dir:    ${INCLUDE_DIR}")
message(STATUS "Tests dir:      ${TEST_DIR}")

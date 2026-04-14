cmake_minimum_required(VERSION 3.20)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

project(libparallax VERSION 0.1.0 LANGUAGES CXX)

message(STATUS "Configuring ${PROJECT_NAME}...")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}" "${CMAKE_BINARY_DIR}/dependencies" "${CMAKE_SOURCE_DIR}/cmake" "${CMAKE_SOURCE_DIR}/cmake/modules")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}" "${CMAKE_BINARY_DIR}/dependencies")

option(PARALLAX_BUILD_DEMO "Build demo" ON)
message(STATUS "PARALLAX_BUILD_DEMO: ${PARALLAX_BUILD_DEMO}")

option(PARALLAX_BUILD_SHARED "Build shared library" OFF)
message(STATUS "PARALLAX_BUILD_SHARED: ${PARALLAX_BUILD_SHARED}")

option(PARALLAX_BUILD_TESTS "Build tests" ON)
message(STATUS "PARALLAX_BUILD_TESTS: ${PARALLAX_BUILD_TESTS}")

option(PARALLAX_BUILD_BENCHMARKS "Build benchmarks" ON)
message(STATUS "PARALLAX_BUILD_BENCHMARKS: ${PARALLAX_BUILD_BENCHMARKS}")

if(${PARALLAX_BUILD_TESTS})
  enable_testing()
endif()

set(_USE_PROFILER_DEFAULT OFF)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(_USE_PROFILER_DEFAULT ON)
endif()

option(PARALLAX_USE_PROFILER "Use profiler" ${_USE_PROFILER_DEFAULT})
message(STATUS "PARALLAX_USE_PROFILER: ${PARALLAX_USE_PROFILER}")

set(_MEMORY_TRACKING OFF)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(_MEMORY_TRACKING ON)
endif()

option(PARALLAX_MEMORY_TRACKING "Use CPU memory tracking" ${_MEMORY_TRACKING})
message(STATUS "PARALLAX_MEMORY_TRACKING: ${PARALLAX_MEMORY_TRACKING}")

set(_USE_ASAN_DEFAULT OFF)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(_USE_ASAN_DEFAULT ON)
endif()

option(PARALLAX_USE_ASAN "Use ASAN" ${_USE_ASAN_DEFAULT})
message(STATUS "PARALLAX_USE_ASAN: ${PARALLAX_USE_ASAN}")

# Get current commit hash
find_package(Git QUIET)

if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE PARALLAX_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE PARALLAX_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
else()
  set(PARALLAX_COMMIT_HASH "unknown")
  set(PARALLAX_BRANCH "unknown")
endif()

add_compile_definitions(
  PARALLAX_COMMIT_HASH="${PARALLAX_COMMIT_HASH}"
  PARALLAX_BRANCH="${PARALLAX_BRANCH}"
)

message(STATUS "Git branch: ${PARALLAX_BRANCH}")
message(STATUS "Git commit: ${PARALLAX_COMMIT_HASH}")

string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPER)

# configure_file("${PROJECT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}/${PROJECT_NAME}/version.hpp.in" "${PROJECT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}/${PROJECT_NAME}/version.hpp" @ONLY)

if(${PARALLAX_BUILD_SHARED})
  add_library(${PROJECT_NAME} SHARED)
else()
  add_library(${PROJECT_NAME} STATIC)
endif()

add_library(${CMAKE_PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

find_package(fmt CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(range-v3 CONFIG REQUIRED)

target_sources(
  ${PROJECT_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}/core/core.cpp"
    "${PROJECT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}/core/move.cpp"
    "${PROJECT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}/core/position.cpp"
    "${PROJECT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}/movegen/movegen.cpp"
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${PROJECT_SOURCE_DIR}
    FILES
      "${PROJECT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}/core/core.hpp"
      "${PROJECT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}/core/move.hpp"
      "${PROJECT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}/core/position.hpp"
      "${PROJECT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}/eval/eval.hpp"
      "${PROJECT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}/movegen/movegen.hpp"
      "${PROJECT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}/perft/perft.hpp"
      "${PROJECT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}/search/search.hpp"
      "${PROJECT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}/uci/uci.hpp"
)

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC
    fmt::fmt
    yaml-cpp::yaml-cpp
    range-v3::range-v3
)

set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    POSITION_INDEPENDENT_CODE ON
)

target_compile_features(
  ${PROJECT_NAME}
  PUBLIC
    cxx_std_23
)

target_compile_options(
  ${PROJECT_NAME}
  PUBLIC
    -Wall 
    -Wextra
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Wcast-align
    -Wunused
    -Woverloaded-virtual
    -Wpedantic
    -Wconversion
    -Wsign-conversion
    -Wnull-dereference
    -Wdouble-promotion
    -Wformat=2
    -Wduplicated-cond
    -Wduplicated-branches
    -Wlogical-op
    -Wuseless-cast
)

install(
  TARGETS
    ${PROJECT_NAME}
  EXPORT
    libPARALLAXTargets
  LIBRARY 
    DESTINATION lib
  ARCHIVE 
    DESTINATION lib
  RUNTIME 
    DESTINATION bin
  FILE_SET 
    HEADERS 
)

set(_LINK_OPTIONS)
set(_COMPILE_OPTIONS)

if(${PARALLAX_USE_ASAN} AND NOT MINGW)
  list(APPEND _COMPILE_OPTIONS -fsanitize=address,undefined -fno-omit-frame-pointer)
  list(APPEND _LINK_OPTIONS -fsanitize=address,undefined)
endif()

if(MINGW)
  list(APPEND _LINK_OPTIONS -Wl,--disable-dynamicbase,--default-image-base-low)
endif()

target_link_options(
  ${PROJECT_NAME}
  PUBLIC
    ${_LINK_OPTIONS}
)

target_compile_options(
  ${PROJECT_NAME}
  PRIVATE
    ${_COMPILE_OPTIONS}
)

if(${PARALLAX_BUILD_TESTS} AND EXISTS "${PROJECT_SOURCE_DIR}/tests")
  add_subdirectory(${PROJECT_SOURCE_DIR}/tests)
endif()

if(${PARALLAX_BUILD_BENCHMARKS} AND EXISTS "${PROJECT_SOURCE_DIR}/benchmarks")
  add_subdirectory(${PROJECT_SOURCE_DIR}/benchmarks)
endif()
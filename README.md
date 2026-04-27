# libparallax

A modern C++23 chess engine library. Provides bitboard-based move generation,
evaluation, alpha-beta search with a transposition table, perft, and a UCI
frontend. Ships as a static or shared library plus an optional UCI demo
executable.

## Features

- Bitboard board representation with Zobrist hashing
- Legal move generation (`libparallax/movegen`)
- Alpha-beta search with transposition table (`libparallax/search`)
- Static evaluation (`libparallax/eval`)
- Perft utility (`libparallax/perft`)
- UCI protocol loop (`libparallax/uci`) usable as a library entry point
- MIT licensed

## Requirements

- C++23 compiler (GCC 13+, Clang 17+, MSVC 19.38+)
- CMake ≥ 3.20
- Conan 2.x for dependency management

Runtime dependencies (fetched via Conan): `fmt`, `spdlog`, `yaml-cpp`,
`nlohmann_json`, `range-v3`, `easy_profiler`, `magic_enum`. Tests use
`gtest`; benchmarks use Google Benchmark.

## Build

```sh
conan install . --build=missing -s build_type=Release
cmake --prefix conan-release
cmake --build --config Release
```

The Conan layout places build artifacts under
`build/<arch>/<compiler>/<build_type>/`. The library is written to
`.../lib/` and the demo binary to `.../bin/`.

### CMake options

| Option                      | Default       | Description                      |
| --------------------------- | ------------- | -------------------------------- |
| `PARALLAX_BUILD_SHARED`     | `OFF`         | Build as a shared library        |
| `PARALLAX_BUILD_DEMO`       | `ON`          | Build the UCI demo executable    |
| `PARALLAX_BUILD_TESTS`      | `ON`          | Build the GoogleTest suite       |
| `PARALLAX_BUILD_BENCHMARKS` | `ON`          | Build Google Benchmark targets   |
| `PARALLAX_USE_ASAN`         | `ON` in Debug | Enable AddressSanitizer + UBSan  |
| `PARALLAX_USE_PROFILER`     | `ON` in Debug | Enable easy_profiler integration |
| `PARALLAX_MEMORY_TRACKING`  | `ON` in Debug | Enable CPU memory tracking       |

Pass as `-o parallax/*:build_tests=False` via Conan or
`-D PARALLAX_BUILD_TESTS=OFF` via CMake.

## Running the demo

The demo is a minimal UCI engine:

```sh
./build/.../bin/demo
```

It reads UCI commands from stdin and writes responses to stdout, so it
can be plugged into any UCI-compatible GUI (Arena, Cute Chess, etc.).

## Tests

```sh
ctest --test-dir build/<arch>/<compiler>/release --output-on-failure
```

## Library usage

Link against the CMake target `libparallax::libparallax`:

```cmake
find_package(libparallax CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE libparallax::libparallax)
```

Minimal search example:

```cpp
#include <chrono>
#include <libparallax/core/position.hpp>
#include <libparallax/search/search.hpp>

auto pos = parallax::position::from_fen(
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1").value();

parallax::search_limits limits{
  .max_time  = std::chrono::milliseconds{1000},
  .max_depth = 12,
};

auto result = parallax::search(pos, limits);
```

To run the UCI loop from your own binary:

```cpp
#include <iostream>
#include <libparallax/uci/uci.hpp>

auto main() -> int {
  return parallax::run_uci_loop(std::cin, std::cout);
}
```

## Project layout

```
libparallax/
  core/       position, bitboard, move, zobrist
  movegen/    legal move generation
  eval/       static evaluation
  search/     alpha-beta + transposition table
  perft/      perft driver
  uci/        UCI protocol loop
demo/         standalone UCI executable
tests/        GoogleTest suite
```

## License

MIT. Copyright (c) 2026 KAJDev. See [LICENSE](LICENSE).

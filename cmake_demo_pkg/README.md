# cmake_demo_pkg

Minimal CMake package example for `find_package` consumption.

This directory is a package-enabled variant of `cmake_demo`.
The original `cmake_demo` is intentionally kept unchanged.

## What this version demonstrates

- Producer (`cmake_demo_pkg`) installs an exported CMake package (`MathLib`).
- Consumer (`consumer_project`) uses `find_package(MathLib CONFIG REQUIRED)`.
- Consumer links `MathLib::math_lib` directly.
- Consumer does **not** manually set include paths or static library file paths.

## Why this solves the previous problem

With manual linking to `libmath_lib.a`, the consumer had to hardcode both:

- include root (for headers)
- library file path

After package export, usage requirements are carried by the imported target.
So include directories and link info are propagated through `MathLib::math_lib`.

## About BUILD_INTERFACE and INSTALL_INTERFACE

- `BUILD_INTERFACE`: used when building against source/build tree.
- `INSTALL_INTERFACE`: used when the target is consumed from installed package files.

`INSTALL_INTERFACE` may look unnecessary in a manual-link setup, but it is required for proper package consumption after install.

## Two-command quick start

Run from the `cmake_demo_pkg` directory:

```bash
cmake -S . -B build_pkg -DCMAKE_INSTALL_PREFIX=$HOME/.local/cmake_demo_pkg_install && cmake --build build_pkg -j && cmake --install build_pkg
cmake -S consumer_project -B consumer_project/build -DCMAKE_PREFIX_PATH=$HOME/.local/cmake_demo_pkg_install && cmake --build consumer_project/build -j && ./consumer_project/build/consumer_demo
```

Expected output:

```text
10 + 32 = 42
6 * 9 = 54
```

## Installed package files

After install, package metadata is available under:

- `$HOME/.local/cmake_demo_pkg_install/lib/cmake/MathLib/MathLibConfig.cmake`
- `$HOME/.local/cmake_demo_pkg_install/lib/cmake/MathLib/MathLibConfigVersion.cmake`
- `$HOME/.local/cmake_demo_pkg_install/lib/cmake/MathLib/MathLibTargets.cmake`

## Key files in this demo

- Producer config: `CMakeLists.txt`
- Package config template: `cmake/MathLibConfig.cmake.in`
- Consumer config: `consumer_project/CMakeLists.txt`

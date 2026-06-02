# tsunami-ui

Qt6 desktop application for the **NskTSH** tsunami simulator: load bathymetry
grids, run simulations, view results, and inspect coastal wave histograms /
profiles over a map.

## Place in the NskTSH ecosystem
Front-end for [`tsunami-core`](https://github.com/NskTSH/tsunami-core).
Data / results follow the workspace convention — see
[`tsunami-data/WORKSPACE.md`](https://github.com/NskTSH/tsunami-data/blob/main/WORKSPACE.md).

## Requirements
- C++20 compiler (MSVC 2022 or recent Clang/GCC)
- CMake ≥ 3.20
- Qt6 (Widgets, Network, Concurrent)

## Build
```bash
git submodule update --init --recursive   # fetches extern/tsunami-core
cmake -B build
cmake --build build --config Release
```

## Dependency on tsunami-core
The UI consumes the header-only target [`NskTSH::tsunami-io`](https://github.com/NskTSH/tsunami-core)
from the solver (parsers/formats: `ASCParser`, `ZDataParser`, `DefaultParametersParser`,
`EllipsoidalSource`, `CoordSystem`). `tsunami-core` is tracked as a **git submodule** at
`extern/tsunami-core`; CMake links the target automatically. To build against a different
checkout, pass `-DTSUNAMI_CORE_DIR=<path-to-tsunami-core>`.

## Data & results
Paths resolve from `NSKTSH_DATA` / `NSKTSH_RESULTS` (default `../Data`,
`../Results` — folders one level above the repo). No absolute paths in code.

## License
© NskTSH — internal/private. See `NOTICE`.

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
cmake -B build -DTSUNAMI_CORE_DIR=../tsunami-core/MacCormackSimulation
cmake --build build --config Release
```

## Dependency on tsunami-core
The UI includes header-only parsers from the solver (`ASCParser`, `ZDataParser`,
`DefaultParametersParser`, `EllipsoidalSource`, `CoordSystem`). Provide them via:
- a sibling checkout of `tsunami-core` (default), or
- a git submodule at `extern/tsunami-core`,

and point CMake at it with `-DTSUNAMI_CORE_DIR=<path-to>/MacCormackSimulation`.

> Note: `tsunami-core` is migrated separately; until then set `TSUNAMI_CORE_DIR`
> to a local checkout of the solver sources.

## Data & results
Paths resolve from `NSKTSH_DATA` / `NSKTSH_RESULTS` (default `../tsunami-data`,
`../results`). No absolute paths in code.

## License
© NskTSH — internal/private. See `NOTICE`.

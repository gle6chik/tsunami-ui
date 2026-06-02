cmake_minimum_required(VERSION 3.20)
project(TsunamiSimUI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Concurrent)

# tsunami-core (SWE solver) provides the header-only target NskTSH::tsunami-io
# (parsers/formats). Tracked as a git submodule at extern/tsunami-core; override
# its location with -DTSUNAMI_CORE_DIR=<path> (e.g. a sibling checkout).
set(TSUNAMI_CORE_DIR "${CMAKE_SOURCE_DIR}/extern/tsunami-core" CACHE PATH
    "Path to the tsunami-core repository (git submodule by default)")

if(NOT EXISTS "${TSUNAMI_CORE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "tsunami-core not found at ${TSUNAMI_CORE_DIR}. "
        "Run 'git submodule update --init --recursive' or pass -DTSUNAMI_CORE_DIR=<path>.")
endif()
add_subdirectory(${TSUNAMI_CORE_DIR} ${CMAKE_BINARY_DIR}/tsunami-core)

add_executable(TsunamiSimUI
    # Entry point
    src/main.cpp

    # Application
    src/app/Application.cpp

    # Model layer
    src/model/Project.cpp
    src/model/ParameterSet.cpp
    src/model/GridDataset.cpp
    src/model/ResultDataset.cpp
    src/model/RunSession.cpp

    # IO layer
    src/io/AsyncGridLoader.cpp
    src/io/AsyncResultLoader.cpp
    src/io/BinaryCache.cpp

    # Geo layer (Reprojection is header-only)
    src/geo/TileProvider.cpp

    # UI layer
    src/ui/MainWindow.cpp
    src/ui/ParameterEditorWidget.cpp
    src/ui/GridViewerWidget.cpp
    src/ui/GridScene.cpp
    src/ui/GradientEditor.cpp
    src/ui/CoastHistogramTool.cpp
    src/ui/ProfileTool.cpp
    src/ui/AnimationPlayer.cpp
    src/ui/SimulationRunnerWidget.cpp
)

target_include_directories(TsunamiSimUI PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(TsunamiSimUI
    NskTSH::tsunami-io
    Qt6::Widgets
    Qt6::Network
    Qt6::Concurrent
)

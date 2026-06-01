cmake_minimum_required(VERSION 3.20)
project(TsunamiSimUI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Concurrent)

# Path to existing simulator headers (header-only parsers)
set(SIM_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../MacCormackSimulation/MacCormackSimulation")

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
    ${SIM_INCLUDE_DIR}
)

target_link_libraries(TsunamiSimUI
    Qt6::Widgets
    Qt6::Network
    Qt6::Concurrent
)

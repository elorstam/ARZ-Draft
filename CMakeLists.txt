cmake_minimum_required(VERSION 3.28)

project(
    ARZDraft
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_library(arz_core STATIC
    core/units/Length.cpp
    core/objects/DocumentObject.cpp
    core/document/ObjectRegistry.cpp
    core/document/Document.cpp

    geometry/primitives/Point2D.cpp
    geometry/tolerance/GeometryTolerance.cpp

    cad/entities/CadEntity.cpp
    cad/entities/LineEntity.cpp
    cad/entities/GraphicsPropertyResolver.cpp

    cad/layers/Layer.cpp
    cad/layers/LayerTable.cpp
)

target_include_directories(arz_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(arz_draft
    app/main/main.cpp
)

target_link_libraries(arz_draft
    PRIVATE
        arz_core
)

add_executable(arz_core_tests
    tests/core_tests.cpp
)

target_link_libraries(arz_core_tests
    PRIVATE
        arz_core
)

add_test(
    NAME core_tests
    COMMAND arz_core_tests
)

add_executable(arz_layer_tests
    tests/layer_tests.cpp
)

target_link_libraries(arz_layer_tests
    PRIVATE
        arz_core
)

add_test(
    NAME layer_tests
    COMMAND arz_layer_tests
)

add_executable(arz_graphics_tests
    tests/graphics_tests.cpp
)

target_link_libraries(arz_graphics_tests
    PRIVATE
        arz_core
)

add_test(
    NAME graphics_tests
    COMMAND arz_graphics_tests
)

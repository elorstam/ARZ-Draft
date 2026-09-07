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

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_library(arz_core STATIC
    core/units/Length.cpp
    core/objects/DocumentObject.cpp
    core/document/ObjectRegistry.cpp
    core/document/Document.cpp
    core/transactions/TransactionHistory.cpp

    geometry/primitives/Point2D.cpp
    geometry/tolerance/GeometryTolerance.cpp
    geometry/algorithms/Distance2D.cpp

    cad/entities/CadEntity.cpp
    cad/entities/LineEntity.cpp
    cad/entities/GraphicsPropertyResolver.cpp

    cad/commands/AddLineCommand.cpp
    cad/commands/AddLinesCommand.cpp
    cad/commands/DeleteEntityCommand.cpp
    cad/commands/DeleteEntitiesCommand.cpp
    cad/commands/MoveEntityCommand.cpp

    cad/spatial/LinearSpatialIndex2D.cpp
    cad/spatial/SpatialIndexSynchronizer.cpp

    cad/selection/LineEntityPickRefiner.cpp
    cad/selection/SelectionService.cpp

    cad/snapping/LineEntitySnapProvider.cpp
    cad/snapping/SnapService.cpp

    cad/layers/Layer.cpp
    cad/layers/LayerTable.cpp
)

add_library(arz_application STATIC
    app/application/CadApplicationController.cpp
    interaction/commands/CommandLineParser.cpp
    interaction/commands/CommandRegistry.cpp
    interaction/commands/CommandInputState.cpp
    interaction/selection/SelectionSet.cpp
    interaction/clipboard/CadClipboard.cpp
    interaction/drafting/DraftingSettings.cpp
    interaction/overlays/OverlayState.cpp
    interaction/tools/LineInputController.cpp
    rendering/Viewport2D.cpp
    rendering/ScreenLineWeightPolicy.cpp
    rendering/LineRenderAdapter.cpp
    rendering/DocumentRenderPlanner.cpp
)

target_include_directories(arz_application
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(arz_application
    PUBLIC
        arz_core
)

target_include_directories(arz_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(arz_draft
    app/main/main.cpp
    app/ui/CadRibbonWidget.cpp
    app/ui/MainWindow.cpp
    rendering/QtDocumentRenderer.cpp
    rendering/QtOverlayRenderer.cpp
    views/CadCanvasWidget.cpp
)

target_link_libraries(arz_draft
    PRIVATE
        arz_application
        Qt6::Widgets
)

if(WIN32)
    get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIRECTORY "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
    find_program(
        QT_WINDEPLOYQT_EXECUTABLE
        windeployqt
        HINTS "${QT_BIN_DIRECTORY}"
        REQUIRED
    )

    add_custom_command(
        TARGET arz_draft
        POST_BUILD
        COMMAND "${QT_WINDEPLOYQT_EXECUTABLE}"
            "$<IF:$<CONFIG:Debug>,--debug,--release>"
            --no-translations
            "$<TARGET_FILE:arz_draft>"
        COMMENT "Deploying Qt runtime dependencies for arz_draft"
        VERBATIM
    )
endif()

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

add_executable(arz_transaction_tests
    tests/transaction_tests.cpp
)

target_link_libraries(arz_transaction_tests
    PRIVATE
        arz_core
)

add_test(
    NAME transaction_tests
    COMMAND arz_transaction_tests
)

add_executable(arz_command_tests
    tests/command_tests.cpp
)

target_link_libraries(arz_command_tests
    PRIVATE
        arz_core
)

add_test(
    NAME command_tests
    COMMAND arz_command_tests
)

add_executable(arz_selection_tests
    tests/selection_tests.cpp
)

target_link_libraries(arz_selection_tests
    PRIVATE
        arz_core
)

add_test(
    NAME selection_tests
    COMMAND arz_selection_tests
)

add_executable(arz_snap_tests
    tests/snap_tests.cpp
)

target_link_libraries(arz_snap_tests
    PRIVATE
        arz_core
)

add_test(
    NAME snap_tests
    COMMAND arz_snap_tests
)

add_executable(arz_interactive_foundation_tests
    tests/interactive_foundation_tests.cpp
)

target_link_libraries(arz_interactive_foundation_tests
    PRIVATE
        arz_application
)

add_test(
    NAME interactive_foundation_tests
    COMMAND arz_interactive_foundation_tests
)

add_executable(arz_cad_interaction_tests
    tests/cad_interaction_tests.cpp
)
target_link_libraries(arz_cad_interaction_tests PRIVATE arz_application)
add_test(NAME cad_interaction_tests COMMAND arz_cad_interaction_tests)

add_executable(arz_rendering_foundation_tests
    tests/rendering_foundation_tests.cpp
)
target_link_libraries(arz_rendering_foundation_tests PRIVATE arz_application)
add_test(NAME rendering_foundation_tests COMMAND arz_rendering_foundation_tests)

add_executable(arz_ui_shell_tests
    app/ui/CadRibbonWidget.cpp
    tests/ui_shell_tests.cpp
)

target_include_directories(arz_ui_shell_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(arz_ui_shell_tests
    PRIVATE
        Qt6::Widgets
)

add_dependencies(arz_ui_shell_tests arz_draft)

add_test(
    NAME ui_shell_tests
    COMMAND arz_ui_shell_tests
)

add_executable(arz_ui_input_routing_tests
    app/ui/CadRibbonWidget.cpp
    app/ui/MainWindow.cpp
    rendering/QtDocumentRenderer.cpp
    rendering/QtOverlayRenderer.cpp
    views/CadCanvasWidget.cpp
    tests/ui_input_routing_tests.cpp
)
target_include_directories(arz_ui_input_routing_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(arz_ui_input_routing_tests PRIVATE arz_application Qt6::Widgets)
add_dependencies(arz_ui_input_routing_tests arz_draft)
add_test(NAME ui_input_routing_tests COMMAND arz_ui_input_routing_tests)

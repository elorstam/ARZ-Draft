#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QFrame>
#include <QTabBar>
#include <QToolButton>

#include "app/ui/CadRibbonWidget.h"

namespace {

bool expect(bool condition, std::string_view message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
    }
    return condition;
}

}

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);

    QAction exitAction(QStringLiteral("Exit"), &application);
    QAction lineAction(QStringLiteral("Line"), &application);
    QAction undoAction(QStringLiteral("Undo"), &application);
    QAction redoAction(QStringLiteral("Redo"), &application);

    arz::app::CadRibbonWidget ribbon({
        &exitAction,
        &lineAction,
        &undoAction,
        &redoAction
    });

    bool passed = true;
    const auto* tabs = ribbon.findChild<QTabBar*>(QStringLiteral("ribbonTabBar"));
    passed &= expect(tabs != nullptr, "Ribbon tab bar exists");
    passed &= expect(tabs != nullptr && tabs->count() == 7, "Ribbon has seven primary tabs");
    passed &= expect(tabs != nullptr && tabs->tabText(0) == QStringLiteral("Home"), "Home is the first ribbon tab");
    passed &= expect(tabs != nullptr && tabs->tabText(6) == QStringLiteral("Output"), "Output is the final ribbon tab");

    for (const auto* groupName : {
        "Draw", "Modify", "Annotation", "Layers", "Block",
        "Properties", "Groups", "Utilities", "Clipboard", "View"
    }) {
        passed &= expect(
            ribbon.findChild<QFrame*>(
                QStringLiteral("ribbonGroup%1").arg(QString::fromLatin1(groupName))
            ) != nullptr,
            std::string("Ribbon group exists: ") + groupName
        );
    }

    const auto* lineButton = ribbon.findChild<QToolButton*>(QStringLiteral("lineToolButton"));
    passed &= expect(lineButton != nullptr, "Line tool exists");
    passed &= expect(lineButton != nullptr && lineButton->isEnabled(), "Line tool remains enabled");
    passed &= expect(lineButton != nullptr && lineButton->defaultAction() == &lineAction, "Line tool uses the functional action");

    for (const auto* toolName : {
        "polylineToolButton", "circleToolButton", "arcToolButton",
        "moveToolButton", "copyToolButton", "rotateToolButton",
        "mirrorToolButton", "trimToolButton", "filletToolButton",
        "stretchToolButton"
    }) {
        const auto* button = ribbon.findChild<QToolButton*>(QString::fromLatin1(toolName));
        passed &= expect(button != nullptr, std::string("Placeholder exists: ") + toolName);
        passed &= expect(button != nullptr && !button->isEnabled(), std::string("Placeholder is disabled: ") + toolName);
    }

    passed &= expect(ribbon.layerSelector() != nullptr, "Layer selector exists");
    for (const auto* selectorName : {
        "colorSelector", "linetypeSelector", "lineweightSelector"
    }) {
        const auto* selector = ribbon.findChild<QComboBox*>(QString::fromLatin1(selectorName));
        passed &= expect(selector != nullptr, std::string("Property selector exists: ") + selectorName);
    }

    if (!passed) {
        return EXIT_FAILURE;
    }

    std::cout << "UI shell tests passed\n";
    return EXIT_SUCCESS;
}

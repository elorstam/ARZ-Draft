#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QFrame>
#include <QLineEdit>
#include <QStackedWidget>
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
    QAction polylineAction(QStringLiteral("Polyline"), &application);
    QAction circleAction(QStringLiteral("Circle"), &application);
    QAction arcAction(QStringLiteral("Arc"), &application);
    QAction undoAction(QStringLiteral("Undo"), &application);
    QAction redoAction(QStringLiteral("Redo"), &application);
    QAction copySelectionAction(QStringLiteral("Copy"), &application);
    QAction cutAction(QStringLiteral("Cut"), &application);
    QAction copyAction(QStringLiteral("Copy"), &application);
    QAction pasteAction(QStringLiteral("Paste"), &application);

    arz::app::CadRibbonWidget ribbon({
        &exitAction,
        &lineAction,
        &polylineAction,
        &circleAction,
        &arcAction,
        &undoAction,
        &redoAction,
        &copySelectionAction,
        &cutAction,
        &copyAction,
        &pasteAction
    });

    bool passed = true;
    auto* tabs = ribbon.findChild<QTabBar*>(QStringLiteral("ribbonTabBar"));
    passed &= expect(tabs != nullptr, "Ribbon tab bar exists");
    passed &= expect(ribbon.height() == arz::app::RibbonMetrics::TotalHeight,
                     "Ribbon uses centralized compact total height");
    passed &= expect(arz::app::RibbonMetrics::LargeIcon < 32
                     && arz::app::RibbonMetrics::SmallIcon <= 16,
                     "Ribbon uses compact centralized icon metrics");
    passed &= expect(tabs != nullptr && tabs->count() == 11, "Ribbon has eleven CAD workspace tabs");
    passed &= expect(tabs != nullptr && tabs->tabText(0) == QStringLiteral("Home"), "Home is the first ribbon tab");
    passed &= expect(tabs != nullptr && tabs->tabText(6) == QStringLiteral("Output"), "Output tab exists");
    passed &= expect(tabs != nullptr && tabs->tabText(10) == QStringLiteral("Featured Apps"), "Featured Apps is the final ribbon tab");

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

    for (const auto& [toolName, action] : {
        std::pair{"polylineToolButton", &polylineAction},
        std::pair{"circleToolButton", &circleAction},
        std::pair{"arcToolButton", &arcAction}
    }) {
        const auto* button = ribbon.findChild<QToolButton*>(QString::fromLatin1(toolName));
        passed &= expect(button != nullptr && button->isEnabled(), std::string("Drawing tool is enabled: ") + toolName);
        passed &= expect(button != nullptr && button->defaultAction() == action, std::string("Drawing tool uses its action: ") + toolName);
    }

    for (const auto* toolName : {
        "moveToolButton", "rotateToolButton",
        "mirrorToolButton", "trimToolButton", "filletToolButton",
        "stretchToolButton", "scaleToolButton", "arrayToolButton"
    }) {
        const auto* button = ribbon.findChild<QToolButton*>(QString::fromLatin1(toolName));
        passed &= expect(button != nullptr, std::string("Placeholder exists: ") + toolName);
        passed &= expect(button != nullptr && !button->isEnabled(), std::string("Placeholder is disabled: ") + toolName);
    }
    const auto* cadCopy = ribbon.findChild<QToolButton*>(QStringLiteral("copyToolButton"));
    passed &= expect(cadCopy != nullptr && cadCopy->defaultAction() == &copySelectionAction,
                     "Modify Copy uses the functional CAD COPY action");

    for (const auto& [toolName, action] : {
        std::pair{"cutToolButton", &cutAction},
        std::pair{"clipboardCopyToolButton", &copyAction},
        std::pair{"pasteToolButton", &pasteAction}
    }) {
        const auto* button = ribbon.findChild<QToolButton*>(QString::fromLatin1(toolName));
        passed &= expect(button != nullptr && button->defaultAction() == action,
                         std::string("Clipboard tool uses its action: ") + toolName);
    }

    passed &= expect(ribbon.layerSelector() != nullptr, "Layer selector exists");
    passed &= expect(ribbon.findChild<QLineEdit*>(QStringLiteral("ribbonSearch")) != nullptr,
                     "Header command search exists");
    const auto* documentTabs = ribbon.findChild<QTabBar*>(QStringLiteral("documentTabBar"));
    passed &= expect(documentTabs != nullptr && documentTabs->count() == 2,
                     "Start and drawing document tabs exist");
    passed &= expect(documentTabs != nullptr && documentTabs->currentIndex() == 1,
                     "Drawing tab is active");
    const auto* pages = ribbon.findChild<QStackedWidget*>(QStringLiteral("ribbonPages"));
    if (tabs && pages) {
        tabs->setCurrentIndex(8);
        passed &= expect(pages->currentIndex() == 8,
                         "Non-Home ribbon tabs switch to structured pages");
    }
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

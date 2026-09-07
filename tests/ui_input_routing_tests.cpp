#include <cstdlib>
#include <iostream>

#include <QApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>

#include "app/ui/MainWindow.h"
#include "views/CadCanvasWidget.h"

namespace {

void press(QWidget& target, int key, const QString& text = {},
           Qt::KeyboardModifiers modifiers = Qt::NoModifier) {
    QKeyEvent event(QEvent::KeyPress, key, modifiers, text);
    QApplication::sendEvent(&target, &event);
    QApplication::processEvents();
}

}

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);
    arz::app::MainWindow window;
    window.show();
    QApplication::processEvents();
    auto* editor = window.findChild<QLineEdit*>(QStringLiteral("commandInput"));
    auto* canvasWidget = window.findChild<QWidget*>(QStringLiteral("cadCanvas"));
    auto* canvas = static_cast<arz::views::CadCanvasWidget*>(canvasWidget);
    if (!editor || !canvas) return EXIT_FAILURE;

    editor->setFocus();
    press(*editor, Qt::Key_A, QStringLiteral("a"));
    press(*editor, Qt::Key_Space, QStringLiteral(" "));
    press(*editor, Qt::Key_B, QStringLiteral("b"));
    press(*editor, Qt::Key_Backspace);
    if (editor->text() != QStringLiteral("a ")) {
        std::cerr << "Text editor did not retain native editing behavior\n";
        return EXIT_FAILURE;
    }
    editor->clear();
    canvas->setFocus();
    for (const auto pair : {std::pair{Qt::Key_L, "l"}, std::pair{Qt::Key_I, "i"},
                            std::pair{Qt::Key_N, "n"}, std::pair{Qt::Key_E, "e"}})
        press(*canvas, pair.first, QString::fromLatin1(pair.second));
    if (editor->text() != QStringLiteral("LINE")) {
        std::cerr << "Canvas typing did not reach shared command buffer\n";
        return EXIT_FAILURE;
    }
    press(*canvas, Qt::Key_Return);
    const auto linePromptVisible = [&window]() {
        for (const auto* label : window.findChildren<QLabel*>()) {
            if (label->text().contains(QStringLiteral("Specify first point"))) return true;
        }
        return false;
    };
    if (!linePromptVisible()) return EXIT_FAILURE;
    press(*canvas, Qt::Key_Return);
    if (linePromptVisible()) return EXIT_FAILURE;
    press(*canvas, Qt::Key_Space, QStringLiteral(" "));
    if (!linePromptVisible()) return EXIT_FAILURE;
    press(*canvas, Qt::Key_Space, QStringLiteral(" "));
    if (linePromptVisible()) return EXIT_FAILURE;
    std::cout << "UI input routing tests passed\n";
    return EXIT_SUCCESS;
}

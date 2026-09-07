#include <iostream>

#include <QApplication>
#include <QTimer>

#include "app/ui/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);
    application.setApplicationName(QStringLiteral("ARZ Studio CAD"));
    application.setOrganizationName(QStringLiteral("ARZ Studio"));
    application.setStyle(QStringLiteral("Fusion"));

    arz::app::MainWindow window;
    window.show();

    if (application.arguments().contains(
            QStringLiteral("--smoke-test"))) {
        QTimer::singleShot(500, &application, &QApplication::quit);
    }

    return application.exec();
}

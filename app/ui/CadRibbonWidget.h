#pragma once

#include <QWidget>

class QAction;
class QComboBox;

namespace arz::app {

struct RibbonActions final {
    QAction* exitAction{nullptr};
    QAction* lineAction{nullptr};
    QAction* undoAction{nullptr};
    QAction* redoAction{nullptr};
};

class CadRibbonWidget final : public QWidget {
public:
    explicit CadRibbonWidget(
        RibbonActions actions,
        QWidget* parent = nullptr
    );

    [[nodiscard]] QComboBox* layerSelector() const noexcept;

private:
    QComboBox* layerSelector_{nullptr};
};

}

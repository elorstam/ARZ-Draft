#pragma once

#include <QWidget>

class QAction;
class QComboBox;

namespace arz::app {

struct RibbonActions final {
    QAction* exitAction{nullptr};
    QAction* lineAction{nullptr};
    QAction* polylineAction{nullptr};
    QAction* circleAction{nullptr};
    QAction* arcAction{nullptr};
    QAction* undoAction{nullptr};
    QAction* redoAction{nullptr};
    QAction* copySelectionAction{nullptr};
    QAction* cutAction{nullptr};
    QAction* copyAction{nullptr};
    QAction* pasteAction{nullptr};
};

struct RibbonMetrics final {
    static constexpr int HeaderHeight = 27;
    static constexpr int TabHeight = 25;
    static constexpr int ContentHeight = 91;
    static constexpr int DocumentTabHeight = 25;
    static constexpr int TotalHeight = HeaderHeight + TabHeight + ContentHeight + DocumentTabHeight;
    static constexpr int LargeIcon = 28;
    static constexpr int SmallIcon = 16;
    static constexpr int GroupHorizontalPadding = 4;
    static constexpr int GroupSpacing = 1;
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

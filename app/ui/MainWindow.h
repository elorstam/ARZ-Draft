#pragma once

#include <QMainWindow>
#include <QPointF>

#include "app/application/CadApplicationController.h"

class QAction;
class QComboBox;
class QLabel;
class QLineEdit;
class QToolButton;
class QEvent;

namespace arz::views {
class CadCanvasWidget;
}

namespace arz::app {

class MainWindow final : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void buildRibbonHeader();
    void buildPropertiesPanel();
    void buildCentralWorkspace();
    void buildStatusBar();
    void executeCommand(const QString& text);
    void startLine();
    void performUndo();
    void performRedo();
    void refreshDraftingToggles();
    void toggleDrafting(arz::interaction::DraftingToggle toggle);
    void refreshUi();
    void refreshProperties();
    void updateCoordinates(arz::geometry::Point2D point);
    void showCanvasContextMenu(QPointF position);

    CadApplicationController controller_;
    arz::views::CadCanvasWidget* canvas_{nullptr};
    QLineEdit* commandInput_{nullptr};
    QLabel* commandPrompt_{nullptr};
    QLabel* coordinateLabel_{nullptr};
    QComboBox* layerSelector_{nullptr};
    QAction* undoAction_{nullptr};
    QAction* redoAction_{nullptr};
    QAction* copySelectionAction_{nullptr};
    QAction* cutAction_{nullptr};
    QAction* copyAction_{nullptr};
    QAction* pasteAction_{nullptr};
    QLabel* objectIdValue_{nullptr};
    QLabel* entityTypeValue_{nullptr};
    QLabel* layerValue_{nullptr};
    QLabel* startXValue_{nullptr};
    QLabel* startYValue_{nullptr};
    QLabel* endXValue_{nullptr};
    QLabel* endYValue_{nullptr};
    QLabel* lengthValue_{nullptr};
    QLabel* verticesValue_{nullptr};
    QLabel* closedValue_{nullptr};
    QLabel* centerXValue_{nullptr};
    QLabel* centerYValue_{nullptr};
    QLabel* radiusValue_{nullptr};
    QLabel* angleValue_{nullptr};
    QToolButton* objectSnapToggle_{nullptr};
    QToolButton* gridToggle_{nullptr};
    QToolButton* orthoToggle_{nullptr};
    QToolButton* gridSnapToggle_{nullptr};
    QToolButton* polarToggle_{nullptr};
    QToolButton* objectSnapTrackingToggle_{nullptr};
    QToolButton* dynamicInputToggle_{nullptr};
    bool synchronizingCommandInput_{false};
};

}

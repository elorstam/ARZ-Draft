#include "app/ui/MainWindow.h"

#include <algorithm>
#include <numbers>

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDockWidget>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QMenu>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QStatusBar>
#include <QTabBar>
#include <QTextEdit>
#include <QToolButton>
#include <QVBoxLayout>
#include <QVariant>

#include "app/ui/CadRibbonWidget.h"
#include "cad/entities/LineEntity.h"
#include "cad/entities/PolylineEntity.h"
#include "cad/entities/CircleEntity.h"
#include "cad/entities/ArcEntity.h"
#include "views/CadCanvasWidget.h"

namespace arz::app {

namespace {

QLabel* propertyValue(const QString& text = QStringLiteral("—")) {
    auto* label = new QLabel(text);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    return label;
}

QToolButton* draftingToggle(const QString& text, bool checked) {
    auto* button = new QToolButton;
    button->setText(text);
    button->setCheckable(true);
    button->setChecked(checked);
    button->setAutoRaise(true);
    return button;
}

bool ownsTextEditing(QWidget* focus) {
    const auto* combo = qobject_cast<QComboBox*>(focus);
    return qobject_cast<QLineEdit*>(focus)
        || qobject_cast<QTextEdit*>(focus)
        || qobject_cast<QPlainTextEdit*>(focus)
        || (combo != nullptr && combo->isEditable());
}

}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("ARZ Studio CAD — CAD Interaction Foundation"));
    resize(1440, 900);
    setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowNestedDocks);
    setStyleSheet(QStringLiteral(
        "QMainWindow { background: #202630; }"
        "QMenuBar, QMenu, QToolBar, QStatusBar { background: #2b333e; color: #e5e9ef; }"
        "QToolBar { border-bottom: 1px solid #151a20; spacing: 6px; }"
        "QDockWidget { color: #e5e9ef; }"
        "QDockWidget::title { background: #303945; padding: 7px; }"
        "QLineEdit, QComboBox { background: #182029; color: #e8edf2; border: 1px solid #536171; padding: 5px; }"
        "QLabel { color: #d8dee7; }"
        "QTabBar::tab { background: #303944; color: #ced6df; padding: 6px 18px; border-right: 1px solid #20262e; }"
        "QTabBar::tab:selected { background: #167d9a; color: white; }"
    ));
    buildRibbonHeader();
    buildPropertiesPanel();
    buildCentralWorkspace();
    buildStatusBar();
    refreshUi();
    qApp->installEventFilter(this);
}

void MainWindow::buildRibbonHeader() {
    auto* exitAction = new QAction(QStringLiteral("E&xit"), this);
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);
    undoAction_ = new QAction(QStringLiteral("&Undo"), this);
    redoAction_ = new QAction(QStringLiteral("&Redo"), this);
    connect(undoAction_, &QAction::triggered, this, [this] { performUndo(); });
    connect(redoAction_, &QAction::triggered, this, [this] { performRedo(); });
    auto* lineAction = new QAction(QStringLiteral("&Line"), this);
    connect(lineAction, &QAction::triggered, this, [this] { startLine(); });
    auto* polylineAction = new QAction(QStringLiteral("&Polyline"), this);
    connect(polylineAction, &QAction::triggered, this, [this] {
        controller_.startPolyline(); canvas_->setFocus(); canvas_->update(); refreshUi();
    });
    auto* circleAction = new QAction(QStringLiteral("&Circle"), this);
    connect(circleAction, &QAction::triggered, this, [this] {
        controller_.startCircle(); canvas_->setFocus(); canvas_->update(); refreshUi();
    });
    auto* arcAction = new QAction(QStringLiteral("&Arc"), this);
    connect(arcAction, &QAction::triggered, this, [this] {
        controller_.startArc(); canvas_->setFocus(); canvas_->update(); refreshUi();
    });
    addAction(lineAction);
    addAction(polylineAction);
    addAction(circleAction);
    addAction(arcAction);
    addAction(undoAction_);
    addAction(redoAction_);

    auto* ribbon = new CadRibbonWidget({exitAction, lineAction, polylineAction,
        circleAction, arcAction, undoAction_, redoAction_});
    setMenuWidget(ribbon);
    layerSelector_ = ribbon->layerSelector();
    auto layerIds = controller_.document().layers().ids();
    std::ranges::sort(layerIds);
    for (const auto layerId : layerIds) {
        const auto* layer = controller_.document().layers().get(layerId);
        if (layer) layerSelector_->addItem(QString::fromStdString(layer->name()),
            QVariant::fromValue<qulonglong>(layerId));
    }
    connect(layerSelector_, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index >= 0) (void)controller_.setCurrentLayerId(layerSelector_->itemData(index).toULongLong());
    });
}

void MainWindow::buildPropertiesPanel() {
    auto* dock = new QDockWidget(QStringLiteral("Properties"), this);
    dock->setObjectName(QStringLiteral("propertiesDock"));
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setMinimumWidth(245);
    auto* panel = new QWidget;
    auto* form = new QFormLayout(panel);
    form->setContentsMargins(12, 14, 12, 14);
    form->setVerticalSpacing(10);
    objectIdValue_ = propertyValue(); entityTypeValue_ = propertyValue();
    layerValue_ = propertyValue(); startXValue_ = propertyValue();
    startYValue_ = propertyValue(); endXValue_ = propertyValue();
    endYValue_ = propertyValue(); lengthValue_ = propertyValue();
    verticesValue_ = propertyValue(); closedValue_ = propertyValue();
    centerXValue_ = propertyValue(); centerYValue_ = propertyValue();
    radiusValue_ = propertyValue(); angleValue_ = propertyValue();
    form->addRow(QStringLiteral("ObjectId"), objectIdValue_);
    form->addRow(QStringLiteral("Entity type"), entityTypeValue_);
    form->addRow(QStringLiteral("Layer"), layerValue_);
    form->addRow(QStringLiteral("Start X"), startXValue_);
    form->addRow(QStringLiteral("Start Y"), startYValue_);
    form->addRow(QStringLiteral("End X"), endXValue_);
    form->addRow(QStringLiteral("End Y"), endYValue_);
    form->addRow(QStringLiteral("Length"), lengthValue_);
    form->addRow(QStringLiteral("Vertices"), verticesValue_);
    form->addRow(QStringLiteral("Closed"), closedValue_);
    form->addRow(QStringLiteral("Center X"), centerXValue_);
    form->addRow(QStringLiteral("Center Y"), centerYValue_);
    form->addRow(QStringLiteral("Radius"), radiusValue_);
    form->addRow(QStringLiteral("Angles"), angleValue_);
    form->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));
    dock->setWidget(panel);
    addDockWidget(Qt::LeftDockWidgetArea, dock);
}

void MainWindow::buildCentralWorkspace() {
    auto* workspace = new QWidget;
    auto* layout = new QVBoxLayout(workspace);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    canvas_ = new arz::views::CadCanvasWidget(controller_);
    canvas_->setObjectName(QStringLiteral("cadCanvas"));
    layout->addWidget(canvas_, 1);
    auto* modelTabs = new QTabBar;
    modelTabs->addTab(QStringLiteral("Model"));
    modelTabs->addTab(QStringLiteral("Layout 1"));
    modelTabs->addTab(QStringLiteral("Layout 2"));
    modelTabs->setExpanding(false);
    layout->addWidget(modelTabs);

    auto* commandFrame = new QFrame;
    commandFrame->setStyleSheet(QStringLiteral("QFrame { background: #242c35; border-top: 1px solid #4b5867; }"));
    auto* commandLayout = new QHBoxLayout(commandFrame);
    commandLayout->setContentsMargins(8, 6, 8, 6);
    commandPrompt_ = new QLabel(QStringLiteral("Command:"));
    commandPrompt_->setMinimumWidth(210);
    commandInput_ = new QLineEdit;
    commandInput_->setObjectName(QStringLiteral("commandInput"));
    commandInput_->setPlaceholderText(QStringLiteral("Type a command, or type directly on the canvas"));
    commandLayout->addWidget(commandPrompt_);
    commandLayout->addWidget(commandInput_, 1);
    layout->addWidget(commandFrame);
    setCentralWidget(workspace);

    canvas_->setCoordinatesChangedCallback([this](auto point) { updateCoordinates(point); });
    canvas_->setStateChangedCallback([this] { refreshUi(); });
    canvas_->setContextMenuCallback([this](QPointF position) {
        showCanvasContextMenu(position);
    });
    connect(commandInput_, &QLineEdit::textChanged, this, [this](const QString& text) {
        if (!synchronizingCommandInput_) {
            controller_.setCommandBuffer(text.toStdString());
            canvas_->update();
        }
    });
    connect(commandInput_, &QLineEdit::returnPressed, this, [this] {
        controller_.setCommandBuffer(commandInput_->text().toStdString());
        (void)controller_.confirmInput();
        commandInput_->clear();
        canvas_->update();
        refreshUi();
    });
}

void MainWindow::buildStatusBar() {
    coordinateLabel_ = new QLabel(QStringLiteral("X: 0.000  Y: 0.000 mm"));
    statusBar()->addWidget(coordinateLabel_, 1);
    objectSnapToggle_ = draftingToggle(QStringLiteral("OSNAP"), true);
    gridToggle_ = draftingToggle(QStringLiteral("GRID"), false);
    orthoToggle_ = draftingToggle(QStringLiteral("ORTHO"), false);
    gridSnapToggle_ = draftingToggle(QStringLiteral("SNAP"), false);
    polarToggle_ = draftingToggle(QStringLiteral("POLAR"), false);
    objectSnapTrackingToggle_ = draftingToggle(QStringLiteral("OTRACK"), false);
    dynamicInputToggle_ = draftingToggle(QStringLiteral("DYN"), true);
    for (auto* button : {objectSnapToggle_, gridToggle_, orthoToggle_, gridSnapToggle_,
                         polarToggle_, objectSnapTrackingToggle_, dynamicInputToggle_})
        statusBar()->addPermanentWidget(button);
    connect(objectSnapToggle_, &QToolButton::clicked, this, [this] { toggleDrafting(arz::interaction::DraftingToggle::ObjectSnap); });
    connect(gridToggle_, &QToolButton::clicked, this, [this] { toggleDrafting(arz::interaction::DraftingToggle::GridDisplay); });
    connect(orthoToggle_, &QToolButton::clicked, this, [this] { toggleDrafting(arz::interaction::DraftingToggle::Ortho); });
    connect(gridSnapToggle_, &QToolButton::clicked, this, [this] { toggleDrafting(arz::interaction::DraftingToggle::GridSnap); });
    connect(polarToggle_, &QToolButton::clicked, this, [this] { toggleDrafting(arz::interaction::DraftingToggle::PolarTracking); });
    connect(objectSnapTrackingToggle_, &QToolButton::clicked, this, [this] { toggleDrafting(arz::interaction::DraftingToggle::ObjectSnapTracking); });
    connect(dynamicInputToggle_, &QToolButton::clicked, this, [this] { toggleDrafting(arz::interaction::DraftingToggle::DynamicInput); });
}

void MainWindow::executeCommand(const QString& text) {
    (void)controller_.invokeCommandText(text.toStdString());
    canvas_->update();
    refreshUi();
}
void MainWindow::startLine() { controller_.startLine(); canvas_->setFocus(); canvas_->update(); refreshUi(); }
void MainWindow::performUndo() { (void)controller_.undo(); canvas_->update(); refreshUi(); }
void MainWindow::performRedo() { (void)controller_.redo(); canvas_->update(); refreshUi(); }

void MainWindow::refreshUi() {
    commandPrompt_->setText(QString::fromStdString(controller_.commandPrompt()));
    synchronizingCommandInput_ = true;
    commandInput_->setText(QString::fromStdString(controller_.commandInput().buffer()));
    synchronizingCommandInput_ = false;
    undoAction_->setEnabled(controller_.history().canUndo());
    redoAction_->setEnabled(controller_.history().canRedo());
    refreshProperties();
    refreshDraftingToggles();
}

void MainWindow::refreshProperties() {
    const auto* entity = dynamic_cast<const arz::cad::CadEntity*>(
        controller_.document().object(controller_.selectedObjectId()));
    const auto* line = dynamic_cast<const arz::cad::LineEntity*>(
        entity);
    const auto clear = [this] {
        for (auto* label : {objectIdValue_, entityTypeValue_, layerValue_, startXValue_,
                            startYValue_, endXValue_, endYValue_, lengthValue_})
            label->setText(QStringLiteral("—"));
    };
    for (auto* label : {verticesValue_, closedValue_, centerXValue_, centerYValue_,
                        radiusValue_, angleValue_})
        label->setText(QStringLiteral("--"));
    if (!entity) {
        clear();
        if (controller_.selection().size() > 1) {
            objectIdValue_->setText(QStringLiteral("%1 objects").arg(controller_.selection().size()));
            entityTypeValue_->setText(QStringLiteral("Multiple selection"));
        }
        return;
    }
    if (!line) {
        clear();
        for (auto* label : {verticesValue_, closedValue_, centerXValue_, centerYValue_,
                            radiusValue_, angleValue_})
            label->setText(QStringLiteral("--"));
        const auto* layer = controller_.document().layers().get(entity->layerId());
        objectIdValue_->setText(QString::number(entity->id()));
        layerValue_->setText(layer ? QString::fromStdString(layer->name()) : QStringLiteral("Unknown"));
        if (const auto* polyline = dynamic_cast<const arz::cad::PolylineEntity*>(entity)) {
            entityTypeValue_->setText(QStringLiteral("Polyline"));
            verticesValue_->setText(QString::number(polyline->vertices().size()));
            closedValue_->setText(polyline->closed() ? QStringLiteral("Yes") : QStringLiteral("No"));
            lengthValue_->setText(QStringLiteral("%1 mm").arg(polyline->length(), 0, 'f', 3));
        } else if (const auto* circle = dynamic_cast<const arz::cad::CircleEntity*>(entity)) {
            entityTypeValue_->setText(QStringLiteral("Circle"));
            centerXValue_->setText(QString::number(circle->center().x, 'f', 3));
            centerYValue_->setText(QString::number(circle->center().y, 'f', 3));
            radiusValue_->setText(QStringLiteral("%1 mm").arg(circle->radius(), 0, 'f', 3));
            lengthValue_->setText(QStringLiteral("%1 mm").arg(circle->circumference(), 0, 'f', 3));
        } else if (const auto* arc = dynamic_cast<const arz::cad::ArcEntity*>(entity)) {
            entityTypeValue_->setText(QStringLiteral("Arc"));
            centerXValue_->setText(QString::number(arc->center().x, 'f', 3));
            centerYValue_->setText(QString::number(arc->center().y, 'f', 3));
            radiusValue_->setText(QStringLiteral("%1 mm").arg(arc->radius(), 0, 'f', 3));
            angleValue_->setText(QStringLiteral("%1 deg to %2 deg").arg(
                arc->startAngle() * 180.0 / std::numbers::pi, 0, 'f', 2).arg(
                arc->endAngle() * 180.0 / std::numbers::pi, 0, 'f', 2));
            lengthValue_->setText(QStringLiteral("%1 mm").arg(arc->length(), 0, 'f', 3));
        }
        return;
    }
    const auto* layer = controller_.document().layers().get(line->layerId());
    objectIdValue_->setText(QString::number(line->id()));
    entityTypeValue_->setText(QStringLiteral("Line"));
    layerValue_->setText(layer ? QString::fromStdString(layer->name()) : QStringLiteral("Unknown"));
    startXValue_->setText(QString::number(line->start().x, 'f', 3));
    startYValue_->setText(QString::number(line->start().y, 'f', 3));
    endXValue_->setText(QString::number(line->end().x, 'f', 3));
    endYValue_->setText(QString::number(line->end().y, 'f', 3));
    lengthValue_->setText(QStringLiteral("%1 mm").arg(line->length(), 0, 'f', 3));
}

void MainWindow::refreshDraftingToggles() {
    using arz::interaction::DraftingToggle;
    const auto& settings = controller_.draftingSettings();
    objectSnapToggle_->setChecked(settings.enabled(DraftingToggle::ObjectSnap));
    gridToggle_->setChecked(settings.enabled(DraftingToggle::GridDisplay));
    orthoToggle_->setChecked(settings.enabled(DraftingToggle::Ortho));
    gridSnapToggle_->setChecked(settings.enabled(DraftingToggle::GridSnap));
    polarToggle_->setChecked(settings.enabled(DraftingToggle::PolarTracking));
    objectSnapTrackingToggle_->setChecked(settings.enabled(DraftingToggle::ObjectSnapTracking));
    dynamicInputToggle_->setChecked(settings.enabled(DraftingToggle::DynamicInput));
}
void MainWindow::toggleDrafting(arz::interaction::DraftingToggle toggle) {
    (void)controller_.toggleDrafting(toggle);
    canvas_->refreshInteractionPointer();
    refreshUi();
}

void MainWindow::updateCoordinates(arz::geometry::Point2D point) {
    coordinateLabel_->setText(QStringLiteral("X: %1  Y: %2 mm")
        .arg(point.x, 0, 'f', 3).arg(point.y, 0, 'f', 3));
}

void MainWindow::showCanvasContextMenu(QPointF position) {
    QMenu menu(this);
    const auto last = controller_.commandInput().lastRepeatable();
    auto* repeat = menu.addAction(last
        ? QStringLiteral("Repeat %1").arg(QString::fromStdString(*last))
        : QStringLiteral("Repeat <Last Command>"));
    repeat->setEnabled(last.has_value());
    if (last) {
        connect(repeat, &QAction::triggered, this, [this, command = *last] {
            (void)controller_.invokeCommandText(command);
            canvas_->update(); refreshUi();
        });
    }

    auto* recent = menu.addMenu(QStringLiteral("Recent Input"));
    const auto& history = controller_.commandInput().history();
    for (auto it = history.rbegin(); it != history.rend(); ++it) {
        auto* action = recent->addAction(QString::fromStdString(*it));
        connect(action, &QAction::triggered, this, [this, command = *it] {
            (void)controller_.invokeCommandText(command);
            canvas_->update(); refreshUi();
        });
    }
    if (history.empty()) recent->addAction(QStringLiteral("<None>"))->setEnabled(false);

    auto* clipboard = menu.addMenu(QStringLiteral("Clipboard"));
    auto* cut = clipboard->addAction(QStringLiteral("Cut"));
    cut->setEnabled(!controller_.selection().empty());
    connect(cut, &QAction::triggered, this, [this] {
        (void)controller_.cutSelection(); canvas_->update(); refreshUi();
    });
    auto* copy = clipboard->addAction(QStringLiteral("Copy"));
    copy->setEnabled(!controller_.selection().empty());
    connect(copy, &QAction::triggered, this, [this] {
        (void)controller_.copySelection(); refreshUi();
    });
    auto* paste = clipboard->addAction(QStringLiteral("Paste"));
    paste->setEnabled(!controller_.clipboard().empty());
    connect(paste, &QAction::triggered, this, [this] {
        (void)controller_.paste(); canvas_->refreshInteractionPointer();
        canvas_->update(); refreshUi();
    });

    menu.addSeparator();
    if (!controller_.selection().empty()) {
        auto* erase = menu.addAction(QStringLiteral("Erase"));
        connect(erase, &QAction::triggered, this, [this] {
            (void)controller_.deleteSelection(); canvas_->update(); refreshUi();
        });
        auto* copySelection = menu.addAction(QStringLiteral("Copy Selection"));
        connect(copySelection, &QAction::triggered, this, [this] {
            (void)controller_.copySelection(); refreshUi();
        });
        menu.addSeparator();
        auto* deselect = menu.addAction(QStringLiteral("Deselect All"));
        connect(deselect, &QAction::triggered, this, [this] {
            (void)controller_.escape(); canvas_->update(); refreshUi();
        });
        menu.addSeparator();
    }
    auto* properties = menu.addAction(QStringLiteral("Properties"));
    connect(properties, &QAction::triggered, this, [this] {
        if (auto* dock = findChild<QDockWidget*>(QStringLiteral("propertiesDock"))) {
            dock->show(); dock->raise(); dock->activateWindow();
        }
    });
    menu.exec(canvas_->mapToGlobal(position.toPoint()));
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() != QEvent::KeyPress || ownsTextEditing(QApplication::focusWidget()))
        return QMainWindow::eventFilter(watched, event);
    auto* key = static_cast<QKeyEvent*>(event);
    const bool control = key->modifiers().testFlag(Qt::ControlModifier);
    bool handled = true;
    if (control && key->key() == Qt::Key_Z) performUndo();
    else if (control && key->key() == Qt::Key_Y) performRedo();
    else if (control && key->key() == Qt::Key_C) { (void)controller_.copySelection(); refreshUi(); }
    else if (control && key->key() == Qt::Key_X) { (void)controller_.cutSelection(); canvas_->update(); refreshUi(); }
    else if (control && key->key() == Qt::Key_V) {
        if (!key->isAutoRepeat() && controller_.paste())
            canvas_->refreshInteractionPointer();
        canvas_->update(); refreshUi();
    }
    else if (control && key->key() == Qt::Key_A) { (void)controller_.selectAll(); canvas_->update(); refreshUi(); }
    else if (key->key() == Qt::Key_Delete) { (void)controller_.deleteSelection(); canvas_->update(); refreshUi(); }
    else if (key->key() == Qt::Key_Escape) { (void)controller_.escape(); canvas_->update(); refreshUi(); }
    else if (key->key() == Qt::Key_Backspace) { controller_.backspaceCommandBuffer(); canvas_->update(); refreshUi(); }
    else if (key->key() == Qt::Key_Up) { controller_.selectPreviousSuggestion(); canvas_->update(); }
    else if (key->key() == Qt::Key_Down) { controller_.selectNextSuggestion(); canvas_->update(); }
    else if (key->key() == Qt::Key_Return || key->key() == Qt::Key_Enter || key->key() == Qt::Key_Space) {
        (void)controller_.confirmInput(); canvas_->update(); refreshUi();
    } else if (key->key() >= Qt::Key_F3 && key->key() <= Qt::Key_F12) {
        using arz::interaction::DraftingToggle;
        switch (key->key()) {
        case Qt::Key_F3: toggleDrafting(DraftingToggle::ObjectSnap); break;
        case Qt::Key_F7: toggleDrafting(DraftingToggle::GridDisplay); break;
        case Qt::Key_F8: toggleDrafting(DraftingToggle::Ortho); break;
        case Qt::Key_F9: toggleDrafting(DraftingToggle::GridSnap); break;
        case Qt::Key_F10: toggleDrafting(DraftingToggle::PolarTracking); break;
        case Qt::Key_F11: toggleDrafting(DraftingToggle::ObjectSnapTracking); break;
        case Qt::Key_F12: toggleDrafting(DraftingToggle::DynamicInput); break;
        default: handled = false; break;
        }
    } else if (!control && !key->text().isEmpty() && key->text().front().isPrint()) {
        controller_.appendCommandCharacter(key->text().front().toLatin1());
        canvas_->update();
        refreshUi();
    } else handled = false;
    return handled || QMainWindow::eventFilter(watched, event);
}

}

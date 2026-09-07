#include "app/ui/CadRibbonWidget.h"

#include <array>
#include <initializer_list>
#include <tuple>

#include <QAction>
#include <QComboBox>
#include <QFrame>
#include <QFont>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QPolygon>
#include <QScrollArea>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QStyle>
#include <QTabBar>
#include <QToolButton>
#include <QVBoxLayout>

namespace arz::app {

namespace {

enum class ToolGlyph {
    Line,
    Polyline,
    Circle,
    Arc,
    Move,
    Copy,
    Rotate,
    Mirror,
    Trim,
    Fillet,
    Stretch,
    Text,
    Dimension,
    Layers,
    Block,
    Group,
    Measure,
    Clipboard,
    View
};

QIcon cadIcon(ToolGlyph glyph) {
    QPixmap pixmap(36, 36);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    const QPen primary(QColor(QStringLiteral("#62c9ef")), 2.2);
    const QPen secondary(QColor(QStringLiteral("#d7e1ea")), 1.7);
    painter.setPen(primary);
    painter.setBrush(Qt::NoBrush);

    switch (glyph) {
    case ToolGlyph::Line:
        painter.drawLine(6, 28, 29, 7);
        painter.setBrush(QColor(QStringLiteral("#f1b84b")));
        painter.drawEllipse(QPointF(6, 28), 2.5, 2.5);
        painter.drawEllipse(QPointF(29, 7), 2.5, 2.5);
        break;
    case ToolGlyph::Polyline: {
        const QPolygon points{{4, 26}, {12, 12}, {22, 24}, {32, 7}};
        painter.drawPolyline(points);
        break;
    }
    case ToolGlyph::Circle:
        painter.drawEllipse(QRectF(6, 6, 24, 24));
        painter.setPen(secondary);
        painter.drawLine(18, 15, 18, 21);
        painter.drawLine(15, 18, 21, 18);
        break;
    case ToolGlyph::Arc:
        painter.drawArc(QRectF(5, 7, 27, 25), 15 * 25, 15 * 205);
        break;
    case ToolGlyph::Move:
        painter.drawLine(18, 4, 18, 32);
        painter.drawLine(4, 18, 32, 18);
        painter.drawLine(18, 4, 14, 9);
        painter.drawLine(18, 4, 22, 9);
        painter.drawLine(32, 18, 27, 14);
        painter.drawLine(32, 18, 27, 22);
        break;
    case ToolGlyph::Copy:
        painter.drawRect(QRectF(6, 10, 18, 19));
        painter.setPen(secondary);
        painter.drawRect(QRectF(12, 5, 18, 19));
        break;
    case ToolGlyph::Rotate:
        painter.drawArc(QRectF(7, 7, 22, 22), 35 * 16, 280 * 16);
        painter.drawLine(27, 7, 29, 14);
        painter.drawLine(27, 7, 20, 8);
        break;
    case ToolGlyph::Mirror:
        painter.setPen(QPen(primary.color(), 1.5, Qt::DashLine));
        painter.drawLine(18, 4, 18, 32);
        painter.setPen(primary);
        painter.drawPolyline(QPolygon{{4, 28}, {14, 8}, {14, 28}});
        painter.setPen(secondary);
        painter.drawPolyline(QPolygon{{32, 28}, {22, 8}, {22, 28}});
        break;
    case ToolGlyph::Trim:
        painter.drawLine(5, 27, 31, 9);
        painter.setPen(secondary);
        painter.drawLine(7, 8, 28, 29);
        painter.setPen(QPen(QColor(QStringLiteral("#f1b84b")), 2));
        painter.drawLine(19, 17, 29, 7);
        break;
    case ToolGlyph::Fillet:
        painter.drawLine(6, 30, 6, 17);
        painter.drawArc(QRectF(6, 6, 24, 24), 90 * 16, -90 * 16);
        painter.drawLine(18, 30, 31, 30);
        break;
    case ToolGlyph::Stretch:
        painter.setPen(QPen(primary.color(), 1.7, Qt::DashLine));
        painter.drawRect(QRectF(5, 8, 17, 20));
        painter.setPen(secondary);
        painter.drawLine(14, 18, 32, 18);
        painter.drawLine(32, 18, 26, 13);
        painter.drawLine(32, 18, 26, 23);
        break;
    case ToolGlyph::Text:
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 21, QFont::DemiBold));
        painter.drawText(QRect(2, 1, 32, 33), Qt::AlignCenter, QStringLiteral("A"));
        break;
    case ToolGlyph::Dimension:
        painter.drawLine(5, 10, 31, 10);
        painter.drawLine(5, 6, 5, 27);
        painter.drawLine(31, 6, 31, 27);
        painter.drawLine(5, 10, 10, 7);
        painter.drawLine(5, 10, 10, 13);
        painter.drawLine(31, 10, 26, 7);
        painter.drawLine(31, 10, 26, 13);
        break;
    case ToolGlyph::Layers:
        for (int offset : {0, 6, 12}) {
            painter.drawPolygon(QPolygon{
                {5 + offset / 2, 11 + offset},
                {18, 5 + offset},
                {31 - offset / 2, 11 + offset},
                {18, 17 + offset}
            });
        }
        break;
    case ToolGlyph::Block:
        painter.drawRect(QRectF(7, 7, 22, 22));
        painter.drawRect(QRectF(12, 12, 12, 12));
        break;
    case ToolGlyph::Group:
        painter.drawEllipse(QRectF(4, 11, 14, 14));
        painter.setPen(secondary);
        painter.drawEllipse(QRectF(18, 11, 14, 14));
        break;
    case ToolGlyph::Measure:
        painter.drawLine(5, 27, 29, 7);
        for (int offset : {0, 6, 12, 18}) {
            painter.drawLine(8 + offset, 24 - offset, 11 + offset, 28 - offset);
        }
        break;
    case ToolGlyph::Clipboard:
        painter.drawRoundedRect(QRectF(7, 7, 22, 25), 2, 2);
        painter.setPen(secondary);
        painter.drawRect(QRectF(12, 4, 12, 6));
        painter.drawLine(12, 17, 24, 17);
        painter.drawLine(12, 23, 22, 23);
        break;
    case ToolGlyph::View:
        painter.drawPolygon(QPolygon{{18, 4}, {31, 11}, {18, 18}, {5, 11}});
        painter.drawPolygon(QPolygon{{5, 11}, {18, 18}, {18, 32}, {5, 25}});
        painter.setPen(secondary);
        painter.drawPolygon(QPolygon{{31, 11}, {18, 18}, {18, 32}, {31, 25}});
        break;
    }

    return QIcon(pixmap);
}

QToolButton* placeholderButton(
    const QString& text,
    ToolGlyph glyph,
    const QString& objectName,
    bool large = false
) {
    auto* button = new QToolButton;
    button->setObjectName(objectName);
    button->setText(text);
    button->setIcon(cadIcon(glyph));
    button->setIconSize(large ? QSize(RibbonMetrics::LargeIcon, RibbonMetrics::LargeIcon)
                              : QSize(RibbonMetrics::SmallIcon, RibbonMetrics::SmallIcon));
    button->setToolButtonStyle(
        large ? Qt::ToolButtonTextUnderIcon : Qt::ToolButtonTextBesideIcon
    );
    button->setEnabled(false);
    button->setProperty("ribbonTool", true);
    button->setProperty("largeTool", large);
    return button;
}

QToolButton* actionButton(
    QAction* action,
    const QString& objectName,
    bool large = false,
    bool showText = false
) {
    auto* button = new QToolButton;
    button->setObjectName(objectName);
    button->setDefaultAction(action);
    button->setIconSize(large ? QSize(RibbonMetrics::LargeIcon, RibbonMetrics::LargeIcon)
                              : QSize(RibbonMetrics::SmallIcon, RibbonMetrics::SmallIcon));
    button->setToolButtonStyle(
        large ? Qt::ToolButtonTextUnderIcon
              : (showText ? Qt::ToolButtonTextBesideIcon : Qt::ToolButtonIconOnly)
    );
    button->setProperty("ribbonTool", true);
    button->setProperty("largeTool", large);
    return button;
}

QFrame* ribbonGroup(
    const QString& title,
    QWidget* content,
    int minimumWidth
) {
    auto* group = new QFrame;
    group->setObjectName(QStringLiteral("ribbonGroup%1").arg(title));
    group->setProperty("ribbonGroup", true);
    group->setMinimumWidth(minimumWidth);

    auto* layout = new QVBoxLayout(group);
    layout->setContentsMargins(RibbonMetrics::GroupHorizontalPadding, 2,
                               RibbonMetrics::GroupHorizontalPadding, 0);
    layout->setSpacing(RibbonMetrics::GroupSpacing);
    layout->addWidget(content, 1);

    auto* caption = new QLabel(title);
    caption->setAlignment(Qt::AlignCenter);
    caption->setProperty("ribbonGroupTitle", true);
    layout->addWidget(caption);
    return group;
}

QWidget* verticalTools(std::initializer_list<QToolButton*> buttons) {
    auto* widget = new QWidget;
    auto* layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(RibbonMetrics::GroupSpacing);
    for (auto* button : buttons) {
        layout->addWidget(button);
    }
    layout->addStretch(1);
    return widget;
}

QComboBox* propertyCombo(
    const QString& text,
    const QString& objectName
) {
    auto* combo = new QComboBox;
    combo->setObjectName(objectName);
    combo->addItem(text);
    combo->setEnabled(false);
    combo->setMinimumWidth(126);
    combo->setMaximumHeight(20);
    return combo;
}

}

CadRibbonWidget::CadRibbonWidget(
    RibbonActions actions,
    QWidget* parent
)
    : QWidget(parent) {
    setObjectName(QStringLiteral("cadRibbon"));
    setFixedHeight(RibbonMetrics::TotalHeight);

    actions.lineAction->setIcon(cadIcon(ToolGlyph::Line));
    actions.polylineAction->setIcon(cadIcon(ToolGlyph::Polyline));
    actions.circleAction->setIcon(cadIcon(ToolGlyph::Circle));
    actions.arcAction->setIcon(cadIcon(ToolGlyph::Arc));
    actions.undoAction->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
    actions.redoAction->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));
    actions.copySelectionAction->setIcon(cadIcon(ToolGlyph::Copy));
    actions.cutAction->setIcon(cadIcon(ToolGlyph::Trim));
    actions.copyAction->setIcon(cadIcon(ToolGlyph::Copy));
    actions.pasteAction->setIcon(cadIcon(ToolGlyph::Clipboard));

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    auto* titleBar = new QFrame;
    titleBar->setObjectName(QStringLiteral("ribbonTitleBar"));
    titleBar->setFixedHeight(RibbonMetrics::HeaderHeight);
    auto* titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(5, 1, 7, 1);
    titleLayout->setSpacing(2);

    auto* applicationButton = new QToolButton;
    applicationButton->setObjectName(QStringLiteral("applicationMenuButton"));
    applicationButton->setText(QStringLiteral("ARZ"));
    applicationButton->setPopupMode(QToolButton::InstantPopup);
    auto* applicationMenu = new QMenu(applicationButton);
    for (const auto& label : {
        QStringLiteral("New Drawing"),
        QStringLiteral("Open Drawing..."),
        QStringLiteral("Save")
    }) {
        auto* action = applicationMenu->addAction(label);
        action->setEnabled(false);
    }
    applicationMenu->addSeparator();
    applicationMenu->addAction(actions.exitAction);
    applicationButton->setMenu(applicationMenu);
    titleLayout->addWidget(applicationButton);

    auto addQuickButton = [titleLayout](
        const QString& name,
        const QIcon& icon,
        const QString& tooltip,
        bool enabled
    ) {
        auto* button = new QToolButton;
        button->setObjectName(name);
        button->setIcon(icon);
        button->setToolTip(tooltip);
        button->setEnabled(enabled);
        button->setAutoRaise(true);
        titleLayout->addWidget(button);
    };
    addQuickButton(
        QStringLiteral("quickNewButton"),
        style()->standardIcon(QStyle::SP_FileIcon),
        QStringLiteral("New"),
        false
    );
    addQuickButton(
        QStringLiteral("quickOpenButton"),
        style()->standardIcon(QStyle::SP_DirOpenIcon),
        QStringLiteral("Open"),
        false
    );
    addQuickButton(
        QStringLiteral("quickSaveButton"),
        style()->standardIcon(QStyle::SP_DialogSaveButton),
        QStringLiteral("Save"),
        false
    );
    titleLayout->addWidget(actionButton(
        actions.undoAction,
        QStringLiteral("quickUndoButton")
    ));
    titleLayout->addWidget(actionButton(
        actions.redoAction,
        QStringLiteral("quickRedoButton")
    ));

    addQuickButton(QStringLiteral("quickSaveAsButton"),
        style()->standardIcon(QStyle::SP_DriveFDIcon), QStringLiteral("Save As"), false);
    addQuickButton(QStringLiteral("quickPlotButton"),
        style()->standardIcon(QStyle::SP_FileDialogDetailedView), QStringLiteral("Plot / Print"), false);

    auto* productTitle = new QLabel(QStringLiteral("ARZ STUDIO CAD"));
    productTitle->setObjectName(QStringLiteral("productTitle"));
    titleLayout->addWidget(productTitle);
    titleLayout->addStretch(1);

    auto* drawingTitle = new QLabel(QStringLiteral("Drawing1.dwg  —  ARZ Studio CAD"));
    drawingTitle->setObjectName(QStringLiteral("drawingTitle"));
    titleLayout->addWidget(drawingTitle);
    titleLayout->addStretch(1);

    auto* search = new QLineEdit;
    search->setObjectName(QStringLiteral("ribbonSearch"));
    search->setPlaceholderText(QStringLiteral("Search commands"));
    search->setFixedSize(168, 20);
    titleLayout->addWidget(search);

    auto utilityButton = [titleLayout](const QString& text, const QString& name, const QString& tip) {
        auto* button = new QToolButton;
        button->setObjectName(name); button->setText(text); button->setToolTip(tip);
        button->setAutoRaise(true); button->setEnabled(false); button->setFixedSize(23, 20);
        titleLayout->addWidget(button);
    };
    utilityButton(QStringLiteral("?"), QStringLiteral("helpButton"), QStringLiteral("Help"));
    utilityButton(QStringLiteral("i"), QStringLiteral("infoButton"), QStringLiteral("Information"));
    utilityButton(QStringLiteral("●"), QStringLiteral("accountButton"), QStringLiteral("Account"));

    auto* workspaceLabel = new QLabel(QStringLiteral("Drafting & Annotation"));
    workspaceLabel->setObjectName(QStringLiteral("workspaceLabel"));
    titleLayout->addWidget(workspaceLabel);
    rootLayout->addWidget(titleBar);

    auto* tabs = new QTabBar;
    tabs->setObjectName(QStringLiteral("ribbonTabBar"));
    tabs->setDrawBase(false);
    tabs->setExpanding(false);
    tabs->setFixedHeight(RibbonMetrics::TabHeight);
    const std::array tabNames{
        QStringLiteral("Home"),
        QStringLiteral("Insert"),
        QStringLiteral("Annotate"),
        QStringLiteral("Parametric"),
        QStringLiteral("View"),
        QStringLiteral("Manage"),
        QStringLiteral("Output"),
        QStringLiteral("Add-ins"),
        QStringLiteral("Collaborate"),
        QStringLiteral("Express Tools"),
        QStringLiteral("Featured Apps")
    };
    for (const auto& tabName : tabNames) {
        tabs->addTab(tabName);
    }
    rootLayout->addWidget(tabs);

    auto* pages = new QStackedWidget;
    pages->setObjectName(QStringLiteral("ribbonPages"));
    pages->setFixedHeight(RibbonMetrics::ContentHeight);

    auto* homePage = new QWidget;
    homePage->setObjectName(QStringLiteral("homeRibbonPage"));
    homePage->setMinimumWidth(1280);
    auto* homeLayout = new QHBoxLayout(homePage);
    homeLayout->setContentsMargins(2, 1, 2, 1);
    homeLayout->setSpacing(0);

    auto* drawContent = new QWidget;
    auto* drawLayout = new QHBoxLayout(drawContent);
    drawLayout->setContentsMargins(0, 0, 0, 0);
    drawLayout->setSpacing(3);
    drawLayout->addWidget(actionButton(
        actions.lineAction,
        QStringLiteral("lineToolButton"),
        true
    ));
    drawLayout->addWidget(verticalTools({
        actionButton(actions.polylineAction, QStringLiteral("polylineToolButton"), false, true),
        actionButton(actions.circleAction, QStringLiteral("circleToolButton"), false, true),
        actionButton(actions.arcAction, QStringLiteral("arcToolButton"), false, true)
    }));
    homeLayout->addWidget(ribbonGroup(QStringLiteral("Draw"), drawContent, 150));

    auto* modifyContent = new QWidget;
    auto* modifyLayout = new QGridLayout(modifyContent);
    modifyLayout->setContentsMargins(0, 0, 0, 0);
    modifyLayout->setHorizontalSpacing(2);
    modifyLayout->setVerticalSpacing(1);
    const std::array modifyTools{
        std::tuple{QStringLiteral("Move"), ToolGlyph::Move, QStringLiteral("moveToolButton")},
        std::tuple{QStringLiteral("Rotate"), ToolGlyph::Rotate, QStringLiteral("rotateToolButton")},
        std::tuple{QStringLiteral("Mirror"), ToolGlyph::Mirror, QStringLiteral("mirrorToolButton")},
        std::tuple{QStringLiteral("Scale"), ToolGlyph::Stretch, QStringLiteral("scaleToolButton")},
        std::tuple{QStringLiteral("Array"), ToolGlyph::Group, QStringLiteral("arrayToolButton")},
        std::tuple{QStringLiteral("Trim"), ToolGlyph::Trim, QStringLiteral("trimToolButton")},
        std::tuple{QStringLiteral("Fillet"), ToolGlyph::Fillet, QStringLiteral("filletToolButton")},
        std::tuple{QStringLiteral("Stretch"), ToolGlyph::Stretch, QStringLiteral("stretchToolButton")}
    };
    for (std::size_t index = 0; index < modifyTools.size(); ++index) {
        const auto& [label, glyph, name] = modifyTools[index];
        modifyLayout->addWidget(
            placeholderButton(label, glyph, name),
            static_cast<int>(index % 3),
            static_cast<int>(index / 3)
        );
    }
    modifyLayout->addWidget(actionButton(actions.copySelectionAction,
        QStringLiteral("copyToolButton"), false, true), 2, 2);
    homeLayout->addWidget(ribbonGroup(QStringLiteral("Modify"), modifyContent, 210));

    auto* annotationContent = new QWidget;
    auto* annotationLayout = new QHBoxLayout(annotationContent);
    annotationLayout->setContentsMargins(0, 0, 0, 0);
    annotationLayout->setSpacing(2);
    annotationLayout->addWidget(placeholderButton(
        QStringLiteral("Text"), ToolGlyph::Text,
        QStringLiteral("textToolButton"), true
    ));
    annotationLayout->addWidget(verticalTools({
        placeholderButton(QStringLiteral("Dimension"), ToolGlyph::Dimension, QStringLiteral("dimensionToolButton")),
        placeholderButton(QStringLiteral("Leader"), ToolGlyph::Dimension, QStringLiteral("leaderToolButton")),
        placeholderButton(QStringLiteral("Table"), ToolGlyph::Block, QStringLiteral("tableToolButton"))
    }));
    homeLayout->addWidget(ribbonGroup(QStringLiteral("Annotation"), annotationContent, 145));

    auto* layersContent = new QWidget;
    auto* layersLayout = new QVBoxLayout(layersContent);
    layersLayout->setContentsMargins(0, 1, 0, 0);
    layersLayout->setSpacing(3);
    auto* layerHeader = new QHBoxLayout;
    auto* layerIcon = new QLabel;
    layerIcon->setPixmap(cadIcon(ToolGlyph::Layers).pixmap(21, 21));
    layerHeader->addWidget(layerIcon);
    layerHeader->addWidget(new QLabel(QStringLiteral("Current layer")), 1);
    layersLayout->addLayout(layerHeader);
    layerSelector_ = new QComboBox;
    layerSelector_->setObjectName(QStringLiteral("layerSelector"));
    layerSelector_->setMinimumWidth(146);
    layersLayout->addWidget(layerSelector_);
    auto* layerState = new QLabel(QStringLiteral("●  On    ❄  Thaw    ◇  Unlock"));
    layerState->setObjectName(QStringLiteral("layerStateSummary"));
    layersLayout->addWidget(layerState);
    layersLayout->addStretch(1);
    homeLayout->addWidget(ribbonGroup(QStringLiteral("Layers"), layersContent, 164));

    auto* blockContent = verticalTools({
        placeholderButton(QStringLiteral("Insert"), ToolGlyph::Block, QStringLiteral("blockToolButton")),
        placeholderButton(QStringLiteral("Create"), ToolGlyph::Block, QStringLiteral("createBlockToolButton")),
        placeholderButton(QStringLiteral("Edit Attr."), ToolGlyph::Block, QStringLiteral("editBlockToolButton"))
    });
    homeLayout->addWidget(ribbonGroup(QStringLiteral("Block"), blockContent, 100));

    auto* propertiesContent = new QWidget;
    auto* propertiesLayout = new QVBoxLayout(propertiesContent);
    propertiesLayout->setContentsMargins(0, 0, 0, 0);
    propertiesLayout->setSpacing(1);
    propertiesLayout->addWidget(placeholderButton(QStringLiteral("Match Properties"),
        ToolGlyph::Copy, QStringLiteral("matchPropertiesToolButton")));
    propertiesLayout->addWidget(propertyCombo(QStringLiteral("ByLayer Color"), QStringLiteral("colorSelector")));
    propertiesLayout->addWidget(propertyCombo(QStringLiteral("Continuous"), QStringLiteral("linetypeSelector")));
    propertiesLayout->addWidget(propertyCombo(QStringLiteral("Default Weight"), QStringLiteral("lineweightSelector")));
    propertiesLayout->addStretch(1);
    homeLayout->addWidget(ribbonGroup(QStringLiteral("Properties"), propertiesContent, 148));

    homeLayout->addWidget(ribbonGroup(
        QStringLiteral("Groups"),
        verticalTools({placeholderButton(QStringLiteral("Group"), ToolGlyph::Group, QStringLiteral("groupToolButton")),
                       placeholderButton(QStringLiteral("Ungroup"), ToolGlyph::Group, QStringLiteral("ungroupToolButton"))}),
        78
    ));
    homeLayout->addWidget(ribbonGroup(
        QStringLiteral("Utilities"),
        verticalTools({placeholderButton(QStringLiteral("Measure"), ToolGlyph::Measure, QStringLiteral("measureToolButton"), true)}),
        70
    ));
    auto* clipboardContent = new QWidget;
    auto* clipboardLayout = new QHBoxLayout(clipboardContent);
    clipboardLayout->setContentsMargins(0, 0, 0, 0);
    clipboardLayout->setSpacing(1);
    clipboardLayout->addWidget(actionButton(actions.pasteAction, QStringLiteral("pasteToolButton"), true));
    clipboardLayout->addWidget(verticalTools({
        actionButton(actions.cutAction, QStringLiteral("cutToolButton"), false, true),
        actionButton(actions.copyAction, QStringLiteral("clipboardCopyToolButton"), false, true)
    }));
    homeLayout->addWidget(ribbonGroup(QStringLiteral("Clipboard"), clipboardContent, 102));
    homeLayout->addWidget(ribbonGroup(
        QStringLiteral("View"),
        verticalTools({placeholderButton(QStringLiteral("Top"), ToolGlyph::View, QStringLiteral("viewToolButton")),
                       placeholderButton(QStringLiteral("Named View"), ToolGlyph::View, QStringLiteral("namedViewToolButton")),
                       placeholderButton(QStringLiteral("2D Wireframe"), ToolGlyph::View, QStringLiteral("visualStyleToolButton"))}),
        96
    ));
    homeLayout->addStretch(1);

    auto* homeScroll = new QScrollArea;
    homeScroll->setObjectName(QStringLiteral("homeRibbonScroll"));
    homeScroll->setFrameShape(QFrame::NoFrame);
    homeScroll->setWidgetResizable(true);
    homeScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    homeScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    homeScroll->setWidget(homePage);
    pages->addWidget(homeScroll);

    for (std::size_t index = 1; index < tabNames.size(); ++index) {
        auto* page = new QWidget;
        page->setObjectName(QStringLiteral("ribbonPlaceholderPage"));
        auto* pageLayout = new QHBoxLayout(page);
        pageLayout->setContentsMargins(2, 1, 2, 1);
        pageLayout->setSpacing(0);
        pageLayout->addWidget(ribbonGroup(
            QStringLiteral("%1 Tools").arg(tabNames[index]),
            verticalTools({placeholderButton(QStringLiteral("Primary Tool"), ToolGlyph::View,
                                              QStringLiteral("placeholderPrimary%1").arg(index), true)}), 104));
        pageLayout->addWidget(ribbonGroup(
            QStringLiteral("Workspace"),
            verticalTools({placeholderButton(QStringLiteral("Options"), ToolGlyph::Layers,
                                              QStringLiteral("placeholderOptions%1").arg(index)),
                           placeholderButton(QStringLiteral("Manager"), ToolGlyph::Block,
                                              QStringLiteral("placeholderManager%1").arg(index))}), 108));
        auto* note = new QLabel(QStringLiteral("Structured %1 workspace — available in a later manual CAD phase")
                                    .arg(tabNames[index]));
        note->setAlignment(Qt::AlignCenter);
        note->setProperty("ribbonPlaceholderNote", true);
        pageLayout->addWidget(note, 1);
        pages->addWidget(page);
    }
    connect(tabs, &QTabBar::currentChanged, pages, &QStackedWidget::setCurrentIndex);
    rootLayout->addWidget(pages);

    auto* documentStrip = new QFrame;
    documentStrip->setObjectName(QStringLiteral("documentTabStrip"));
    documentStrip->setFixedHeight(RibbonMetrics::DocumentTabHeight);
    auto* documentLayout = new QHBoxLayout(documentStrip);
    documentLayout->setContentsMargins(5, 0, 5, 0);
    documentLayout->setSpacing(1);
    auto* documentTabs = new QTabBar;
    documentTabs->setObjectName(QStringLiteral("documentTabBar"));
    documentTabs->setDrawBase(false);
    documentTabs->setExpanding(false);
    documentTabs->addTab(QStringLiteral("Start"));
    documentTabs->addTab(QStringLiteral("Drawing1.dwg"));
    documentTabs->setCurrentIndex(1);
    auto* closeDrawing = new QToolButton;
    closeDrawing->setObjectName(QStringLiteral("closeDrawingTabButton"));
    closeDrawing->setText(QStringLiteral("×"));
    closeDrawing->setToolTip(QStringLiteral("Close drawing"));
    closeDrawing->setEnabled(false);
    closeDrawing->setFixedSize(16, 16);
    documentTabs->setTabButton(1, QTabBar::RightSide, closeDrawing);
    documentLayout->addWidget(documentTabs);
    auto* newTab = new QToolButton;
    newTab->setObjectName(QStringLiteral("newDocumentTabButton"));
    newTab->setText(QStringLiteral("+"));
    newTab->setToolTip(QStringLiteral("New drawing tab"));
    newTab->setEnabled(false);
    newTab->setFixedSize(23, 21);
    documentLayout->addWidget(newTab);
    documentLayout->addStretch(1);
    rootLayout->addWidget(documentStrip);

    setStyleSheet(QStringLiteral(
        "#cadRibbon { background: #252d38; color: #dce5ee; border-bottom: 1px solid #10151b; font-family: 'Segoe UI'; font-size: 9px; }"
        "#ribbonTitleBar { background: #172431; border-bottom: 1px solid #0d141c; }"
        "#applicationMenuButton { background: #167ea0; color: white; border: 0; border-radius: 2px; font-weight: 700; padding: 2px 8px; max-height: 21px; }"
        "#applicationMenuButton:hover { background: #2499bd; }"
        "#productTitle { color: #7ed8f5; font-weight: 700; letter-spacing: 1px; padding-left: 3px; }"
        "#drawingTitle { color: #c4ced8; font-size: 10px; }"
        "#workspaceLabel { color: #b8c8d8; background: #233444; border: 1px solid #3d5266; border-radius: 2px; padding: 1px 6px; }"
        "#ribbonSearch { color: #dce5ee; background: #111a23; border: 1px solid #405267; border-radius: 2px; padding: 1px 6px; font-size: 9px; }"
        "#helpButton, #infoButton, #accountButton { color: #d7e2ec; border: 1px solid transparent; padding: 0; font-weight: 600; }"
        "#ribbonTabBar { background: #202a35; }"
        "#ribbonTabBar::tab { background: transparent; color: #c7d0da; min-width: 58px; padding: 4px 8px 3px 8px; border: 0; font-size: 9px; }"
        "#ribbonTabBar::tab:hover { background: #2d3b49; color: white; }"
        "#ribbonTabBar::tab:selected { color: white; background: #303b47; border-top: 2px solid #42b7dc; }"
        "#ribbonPages, #homeRibbonPage, #homeRibbonScroll, #ribbonPlaceholderPage { background: #303945; color: #aebdcb; border: 0; }"
        "QFrame[ribbonGroup=\"true\"] { background: #303945; border: 0; border-right: 1px solid #56616e; }"
        "QLabel[ribbonGroupTitle=\"true\"] { color: #aab5c0; font-size: 9px; padding: 0 2px 1px 2px; }"
        "QLabel[ribbonPlaceholderNote=\"true\"] { color: #81909f; font-size: 10px; }"
        "QToolButton[ribbonTool=\"true\"] { color: #e1e7ed; background: transparent; border: 1px solid transparent; border-radius: 1px; padding: 1px 3px; min-height: 17px; font-size: 9px; }"
        "QToolButton[ribbonTool=\"true\"]:hover { background: #425262; border-color: #65788a; }"
        "QToolButton[ribbonTool=\"true\"]:pressed { background: #176f8c; }"
        "QToolButton[ribbonTool=\"true\"]:disabled { color: #8f9aa5; }"
        "QToolButton[largeTool=\"true\"] { min-width: 44px; min-height: 55px; }"
        "QComboBox { color: #dce5ee; background: #1e2832; border: 1px solid #566676; border-radius: 1px; padding: 1px 4px; font-size: 9px; }"
        "QComboBox:disabled { color: #a7b1ba; background: #252e38; }"
        "#documentTabStrip { background: #1b232d; border-top: 1px solid #10161d; border-bottom: 1px solid #111820; }"
        "#documentTabBar::tab { background: #27313c; color: #b9c4ce; min-width: 90px; padding: 3px 12px; border: 0; border-right: 1px solid #111820; font-size: 9px; }"
        "#documentTabBar::tab:selected { background: #3a4652; color: white; border-top: 2px solid #47bce0; }"
        "#newDocumentTabButton { color: #b9c4ce; background: transparent; border: 1px solid transparent; font-size: 15px; padding: 0; }"
        "#closeDrawingTabButton { color: #aeb9c4; background: transparent; border: 0; font-size: 12px; padding: 0; }"
        "QMenu { background: #252f3a; color: #e1e7ed; border: 1px solid #526170; }"
        "QMenu::item { padding: 6px 26px; }"
        "QMenu::item:selected { background: #167e9f; }"
    ));
}

QComboBox* CadRibbonWidget::layerSelector() const noexcept {
    return layerSelector_;
}

}

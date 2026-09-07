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
    button->setIconSize(large ? QSize(34, 34) : QSize(20, 20));
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
    bool large = false
) {
    auto* button = new QToolButton;
    button->setObjectName(objectName);
    button->setDefaultAction(action);
    button->setIconSize(large ? QSize(34, 34) : QSize(18, 18));
    button->setToolButtonStyle(
        large ? Qt::ToolButtonTextUnderIcon : Qt::ToolButtonIconOnly
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
    layout->setContentsMargins(6, 4, 6, 0);
    layout->setSpacing(1);
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
    layout->setSpacing(1);
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
    combo->setMinimumWidth(142);
    combo->setMaximumHeight(25);
    return combo;
}

}

CadRibbonWidget::CadRibbonWidget(
    RibbonActions actions,
    QWidget* parent
)
    : QWidget(parent) {
    setObjectName(QStringLiteral("cadRibbon"));
    setFixedHeight(187);

    actions.lineAction->setIcon(cadIcon(ToolGlyph::Line));
    actions.polylineAction->setIcon(cadIcon(ToolGlyph::Polyline));
    actions.circleAction->setIcon(cadIcon(ToolGlyph::Circle));
    actions.arcAction->setIcon(cadIcon(ToolGlyph::Arc));
    actions.undoAction->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
    actions.redoAction->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    auto* titleBar = new QFrame;
    titleBar->setObjectName(QStringLiteral("ribbonTitleBar"));
    titleBar->setFixedHeight(34);
    auto* titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(8, 2, 10, 2);
    titleLayout->setSpacing(4);

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

    auto* productTitle = new QLabel(QStringLiteral("ARZ STUDIO CAD"));
    productTitle->setObjectName(QStringLiteral("productTitle"));
    titleLayout->addWidget(productTitle);
    titleLayout->addStretch(1);

    auto* drawingTitle = new QLabel(QStringLiteral("Drawing 1  |  2D Drafting Workspace"));
    drawingTitle->setObjectName(QStringLiteral("drawingTitle"));
    titleLayout->addWidget(drawingTitle);
    titleLayout->addStretch(1);

    auto* workspaceLabel = new QLabel(QStringLiteral("Drafting & Annotation"));
    workspaceLabel->setObjectName(QStringLiteral("workspaceLabel"));
    titleLayout->addWidget(workspaceLabel);
    rootLayout->addWidget(titleBar);

    auto* tabs = new QTabBar;
    tabs->setObjectName(QStringLiteral("ribbonTabBar"));
    tabs->setDrawBase(false);
    tabs->setExpanding(false);
    tabs->setFixedHeight(29);
    const std::array tabNames{
        QStringLiteral("Home"),
        QStringLiteral("Insert"),
        QStringLiteral("Annotate"),
        QStringLiteral("Parametric"),
        QStringLiteral("View"),
        QStringLiteral("Manage"),
        QStringLiteral("Output")
    };
    for (const auto& tabName : tabNames) {
        tabs->addTab(tabName);
    }
    rootLayout->addWidget(tabs);

    auto* pages = new QStackedWidget;
    pages->setObjectName(QStringLiteral("ribbonPages"));

    auto* homePage = new QWidget;
    homePage->setObjectName(QStringLiteral("homeRibbonPage"));
    homePage->setMinimumWidth(1370);
    auto* homeLayout = new QHBoxLayout(homePage);
    homeLayout->setContentsMargins(4, 2, 4, 2);
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
        actionButton(actions.polylineAction, QStringLiteral("polylineToolButton")),
        actionButton(actions.circleAction, QStringLiteral("circleToolButton")),
        actionButton(actions.arcAction, QStringLiteral("arcToolButton"))
    }));
    homeLayout->addWidget(ribbonGroup(QStringLiteral("Draw"), drawContent, 186));

    auto* modifyContent = new QWidget;
    auto* modifyLayout = new QGridLayout(modifyContent);
    modifyLayout->setContentsMargins(0, 0, 0, 0);
    modifyLayout->setHorizontalSpacing(2);
    modifyLayout->setVerticalSpacing(1);
    const std::array modifyTools{
        std::tuple{QStringLiteral("Move"), ToolGlyph::Move, QStringLiteral("moveToolButton")},
        std::tuple{QStringLiteral("Copy"), ToolGlyph::Copy, QStringLiteral("copyToolButton")},
        std::tuple{QStringLiteral("Rotate"), ToolGlyph::Rotate, QStringLiteral("rotateToolButton")},
        std::tuple{QStringLiteral("Mirror"), ToolGlyph::Mirror, QStringLiteral("mirrorToolButton")},
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
    homeLayout->addWidget(ribbonGroup(QStringLiteral("Modify"), modifyContent, 222));

    auto* annotationContent = new QWidget;
    auto* annotationLayout = new QHBoxLayout(annotationContent);
    annotationLayout->setContentsMargins(0, 0, 0, 0);
    annotationLayout->setSpacing(2);
    annotationLayout->addWidget(placeholderButton(
        QStringLiteral("Text"), ToolGlyph::Text,
        QStringLiteral("textToolButton"), true
    ));
    annotationLayout->addWidget(verticalTools({
        placeholderButton(QStringLiteral("Dimension"), ToolGlyph::Dimension, QStringLiteral("dimensionToolButton"))
    }));
    homeLayout->addWidget(ribbonGroup(QStringLiteral("Annotation"), annotationContent, 154));

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
    layerSelector_->setMinimumWidth(154);
    layersLayout->addWidget(layerSelector_);
    auto* layerState = new QLabel(QStringLiteral("●  On    ❄  Thaw    ◇  Unlock"));
    layerState->setObjectName(QStringLiteral("layerStateSummary"));
    layersLayout->addWidget(layerState);
    layersLayout->addStretch(1);
    homeLayout->addWidget(ribbonGroup(QStringLiteral("Layers"), layersContent, 176));

    auto* blockContent = verticalTools({
        placeholderButton(QStringLiteral("Insert"), ToolGlyph::Block, QStringLiteral("blockToolButton"), true)
    });
    homeLayout->addWidget(ribbonGroup(QStringLiteral("Block"), blockContent, 72));

    auto* propertiesContent = new QWidget;
    auto* propertiesLayout = new QVBoxLayout(propertiesContent);
    propertiesLayout->setContentsMargins(0, 0, 0, 0);
    propertiesLayout->setSpacing(2);
    propertiesLayout->addWidget(propertyCombo(QStringLiteral("ByLayer Color"), QStringLiteral("colorSelector")));
    propertiesLayout->addWidget(propertyCombo(QStringLiteral("Continuous"), QStringLiteral("linetypeSelector")));
    propertiesLayout->addWidget(propertyCombo(QStringLiteral("Default Weight"), QStringLiteral("lineweightSelector")));
    propertiesLayout->addStretch(1);
    homeLayout->addWidget(ribbonGroup(QStringLiteral("Properties"), propertiesContent, 158));

    homeLayout->addWidget(ribbonGroup(
        QStringLiteral("Groups"),
        verticalTools({placeholderButton(QStringLiteral("Group"), ToolGlyph::Group, QStringLiteral("groupToolButton"), true)}),
        72
    ));
    homeLayout->addWidget(ribbonGroup(
        QStringLiteral("Utilities"),
        verticalTools({placeholderButton(QStringLiteral("Measure"), ToolGlyph::Measure, QStringLiteral("measureToolButton"), true)}),
        78
    ));
    homeLayout->addWidget(ribbonGroup(
        QStringLiteral("Clipboard"),
        verticalTools({placeholderButton(QStringLiteral("Paste"), ToolGlyph::Clipboard, QStringLiteral("pasteToolButton"), true)}),
        76
    ));
    homeLayout->addWidget(ribbonGroup(
        QStringLiteral("View"),
        verticalTools({placeholderButton(QStringLiteral("Top"), ToolGlyph::View, QStringLiteral("viewToolButton"), true)}),
        68
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
        auto* placeholder = new QLabel(
            QStringLiteral("%1 tools are reserved for a later manual CAD phase.")
                .arg(tabNames[index])
        );
        placeholder->setObjectName(QStringLiteral("ribbonPlaceholderPage"));
        placeholder->setAlignment(Qt::AlignCenter);
        pages->addWidget(placeholder);
    }
    connect(tabs, &QTabBar::currentChanged, pages, &QStackedWidget::setCurrentIndex);
    rootLayout->addWidget(pages, 1);

    setStyleSheet(QStringLiteral(
        "#cadRibbon { background: #252d38; color: #dce5ee; border-bottom: 1px solid #10151b; }"
        "#ribbonTitleBar { background: #182431; border-bottom: 1px solid #0d141c; }"
        "#applicationMenuButton { background: #167ea0; color: white; border: 0; border-radius: 3px; font-weight: 700; padding: 4px 11px; }"
        "#applicationMenuButton:hover { background: #2499bd; }"
        "#productTitle { color: #7ed8f5; font-weight: 700; letter-spacing: 1px; padding-left: 5px; }"
        "#drawingTitle { color: #b9c5d1; font-size: 11px; }"
        "#workspaceLabel { color: #b8c8d8; background: #233444; border: 1px solid #3d5266; border-radius: 2px; padding: 3px 9px; }"
        "#ribbonTabBar { background: #202a35; }"
        "#ribbonTabBar::tab { background: transparent; color: #c7d0da; min-width: 74px; padding: 6px 12px 5px 12px; border: 0; }"
        "#ribbonTabBar::tab:hover { background: #2d3b49; color: white; }"
        "#ribbonTabBar::tab:selected { color: white; background: #303b47; border-top: 2px solid #42b7dc; }"
        "#ribbonPages, #homeRibbonPage, #homeRibbonScroll, #ribbonPlaceholderPage { background: #303945; color: #aebdcb; border: 0; }"
        "QFrame[ribbonGroup=\"true\"] { background: #303945; border: 0; border-right: 1px solid #56616e; }"
        "QLabel[ribbonGroupTitle=\"true\"] { color: #9eabb8; font-size: 10px; padding: 1px 2px 2px 2px; }"
        "QToolButton[ribbonTool=\"true\"] { color: #e1e7ed; background: transparent; border: 1px solid transparent; border-radius: 2px; padding: 2px 4px; min-height: 20px; }"
        "QToolButton[ribbonTool=\"true\"]:hover { background: #425262; border-color: #65788a; }"
        "QToolButton[ribbonTool=\"true\"]:pressed { background: #176f8c; }"
        "QToolButton[ribbonTool=\"true\"]:disabled { color: #8f9aa5; }"
        "QToolButton[largeTool=\"true\"] { min-width: 54px; min-height: 68px; }"
        "QComboBox { color: #dce5ee; background: #1e2832; border: 1px solid #566676; border-radius: 2px; padding: 3px 6px; }"
        "QComboBox:disabled { color: #a7b1ba; background: #252e38; }"
        "QMenu { background: #252f3a; color: #e1e7ed; border: 1px solid #526170; }"
        "QMenu::item { padding: 6px 26px; }"
        "QMenu::item:selected { background: #167e9f; }"
    ));
}

QComboBox* CadRibbonWidget::layerSelector() const noexcept {
    return layerSelector_;
}

}

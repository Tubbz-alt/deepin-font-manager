#include "views/dfontmgrmainwindow.h"
#include "dfinstallnormalwindow.h"

#include "globaldef.h"
#include "interfaces/dfontmenumanager.h"
#include "utils.h"
#include "views/dfdeletedialog.h"
#include "views/dfontinfodialog.h"
#include "views/dfquickinstallwindow.h"

#include <QHBoxLayout>
#include <QShortcut>
#include <QFileSystemWatcher>

#include <DApplication>
#include <DApplicationHelper>
#include <DFontSizeManager>
#include <DFileDialog>
#include <DIconButton>
#include <DLabel>
#include <DLineEdit>
#include <DLog>
#include <DMenu>
#include <DSearchEdit>
#include <DSlider>
#include <DSplitter>
#include <DTitlebar>
#include <DWidgetUtil>
#include <DDesktopServices>
#include <DMessageManager>
#include <DSpinner>

#include <unistd.h>

class DFontMgrMainWindowPrivate
{
public:
    DFontMgrMainWindowPrivate(DFontMgrMainWindow *q)
        : settingsQsPtr(new QSettings(QDir(Utils::getConfigPath()).filePath("config.conf"),
                                      QSettings::IniFormat))
        , q_ptr(q)
    {
    }

    //~DFontMgrMainWindowPrivate() {}
    QWidget *titleActionArea {nullptr};
    DIconButton *addFontButton {nullptr};
    DSearchEdit *searchFontEdit {nullptr};

    QWidget *fontShowArea {nullptr};

    //Shadow line of StateBar
    DHorizontalLine  *sbarShadowLine {nullptr};

    QWidget *stateBar {nullptr};
    DLineEdit *textInputEdit {nullptr};
    DSlider *fontScaleSlider {nullptr};
    DLabel *fontSizeLabel {nullptr};

    DSplitter *mainWndSpliter {nullptr};
    QWidget *leftBarHolder {nullptr};
    QWidget *rightViewHolder {nullptr};

    // Menu
    DMenu *toolBarMenu {nullptr};
    DMenu *rightKeyMenu {nullptr};

    DSplitListWidget *leftSiderBar {nullptr};

    QScopedPointer<QSettings> settingsQsPtr;
    DFontMgrMainWindow *q_ptr;

    Q_DECLARE_PUBLIC(DFontMgrMainWindow)
};

DFontMgrMainWindow::DFontMgrMainWindow(bool isQuickMode, QWidget *parent)
    : DMainWindow(parent)
    , m_isQuickMode(isQuickMode)
    , m_fontManager(DFontManager::instance())
    , m_scFullScreen(nullptr)
    , m_scZoomIn(nullptr)
    , m_scZoomOut(nullptr)
    , m_scDefaultSize(nullptr)
    , m_previewFontSize(DEFAULT_FONT_SIZE)
    , m_quickInstallWnd(nullptr)
    , d_ptr(new DFontMgrMainWindowPrivate(this))
{
    // setWindowOpacity(0.5); //Debug
    // setWindowFlags(windowFlags() | (Qt::FramelessWindowHint | Qt::WindowMaximizeButtonHint));

    initData();
    initUI();
    initConnections();
    initShortcuts();
    initFontFiles();
    initFontUninstallDialog();
}

DFontMgrMainWindow::~DFontMgrMainWindow()
{
    d_func()->settingsQsPtr->setValue(FTM_MWSIZE_H_KEY, m_winHight);
    d_func()->settingsQsPtr->setValue(FTM_MWSIZE_W_KEY, m_winWidth);
}

void DFontMgrMainWindow::initData()
{
    D_D(DFontMgrMainWindow);

    //Initialize app Theme
    QVariant theme;
    theme = d->settingsQsPtr->value(FTM_THEME_KEY);

    bool ok = false;
    int color = theme.toInt(&ok);

    DGuiApplicationHelper::ColorType colorType = DGuiApplicationHelper::ColorType::UnknownType;

    if (ok) {
        colorType = static_cast<DGuiApplicationHelper::ColorType>(color);
    }

    m_winHight = d->settingsQsPtr->value(FTM_MWSIZE_H_KEY).toInt();
    m_winWidth = d->settingsQsPtr->value(FTM_MWSIZE_W_KEY).toInt();

    qDebug() << __FUNCTION__ << "init theme = " << colorType;

    DGuiApplicationHelper::instance()->setPaletteType(colorType);
}

void DFontMgrMainWindow::initUI()
{
    //Enable main window accept drag event
    setAcceptDrops(true);

    initTileBar();
    initRightKeyMenu();
    initMainVeiws();
}

void DFontMgrMainWindow::initConnections()
{
    D_D(DFontMgrMainWindow);

    // Loading Font List Signal
    QObject::connect(m_fontPreviewListView, SIGNAL(onLoadFontsStatus(int)),
                     this, SLOT(onLoadStatus(int)));

    // Add Font button event
    QObject::connect(d->addFontButton, &DIconButton::clicked, this,
                     &DFontMgrMainWindow::handleAddFontEvent);

    QObject::connect(this, &DFontMgrMainWindow::fileSelected, this,
    [this](const QStringList & files) {
        this->installFont(files);
    });

    QObject::connect(this, &DFontMgrMainWindow::fileSelectedInSys, this,
    [this](const QStringList & files) {
        this->installFontFromSys(files);
    });
    // Menu event
    QObject::connect(d->toolBarMenu, &QMenu::triggered, this, &DFontMgrMainWindow::handleMenuEvent);

    //Theme change event
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::paletteTypeChanged,
    [this](DGuiApplicationHelper::ColorType type) {
        qDebug() << "Update Theme type:" << type;
        //Save theme value
        d_func()->settingsQsPtr->setValue(FTM_THEME_KEY, type);
    });

    // Right Key menu
    QObject::connect(d->rightKeyMenu, &QMenu::triggered, this,
                     &DFontMgrMainWindow::handleMenuEvent);
    // Initialize rigth menu it state
    QObject::connect(d->rightKeyMenu, &QMenu::aboutToShow, this, [ = ]() {
        DFontPreviewItemData currItemData = m_fontPreviewListView->currModelData();
        int cnt = 0;
        int systemCnt = 0;
        m_fontPreviewListView->selectedFonts(&cnt, &systemCnt);

        DFontMenuManager::getInstance()->onRightKeyMenuPopup(currItemData, (cnt > 0));
    });

    // State bar event
    QObject::connect(d->fontScaleSlider, &DSlider::valueChanged, this, [this, d](int value) {
        m_previewFontSize = value;

        QString fontSizeText;
        fontSizeText.sprintf(FMT_FONT_SIZE, value);
        d->fontSizeLabel->setText(fontSizeText);

        onFontSizeChanged(value);
    });

    QObject::connect(m_signalManager, &SignalManager::popUninstallDialog, this, [ this ] {
        qDebug() << "pop uninstall progress dialog ";
        DFontuninstalldialog *dialog = new DFontuninstalldialog(this);
        dialog->move(this->geometry().center() - dialog->rect().center());
        dialog->exec();
    }, Qt::UniqueConnection);

    QObject::connect(m_signalManager, &SignalManager::deledFont, this, [ = ](QString & fontPath) {
        if (m_uninstallFilePath.count() == 0)
            return;
        foreach (auto it, m_uninstallFilePath) {
            if (it == fontPath) {
                m_deledCount++;
                emit SignalManager::instance()->updateUninstallDialog(it.split("/").last(), m_deledCount, m_needDelCount);
                m_uninstallFilePath.removeOne(it);
                break;
            }
        }
        checkCloseUninstallDialog();
    });
    // Search text changed
    QObject::connect(d->searchFontEdit, SIGNAL(textChanged(const QString &)), this,
                     SLOT(onSearchTextChanged(const QString &)));

    QObject::connect(d->textInputEdit, SIGNAL(textChanged(const QString &)), this,
                     SLOT(onPreviewTextChanged(const QString &)));

    QObject::connect(d->leftSiderBar, SIGNAL(onListWidgetItemClicked(int)), this,
                     SLOT(onLeftSiderBarItemClicked(int)));

    QObject::connect(m_fontManager, SIGNAL(uninstallFontFinished(const QStringList &)), this,
                     SLOT(onFontUninstallFinished(const QStringList &)));
    QObject::connect(m_signalManager, &SignalManager::showInstallFloatingMessage, this, &DFontMgrMainWindow::onShowMessage);
    connect(this, &DFontMgrMainWindow::requestUpdatePreview, [ = ] {
        QString previewText = d->textInputEdit->text();
        onPreviewTextChanged(previewText);
    });

    //安装结束后刷新字体列表
    connect(m_signalManager, &SignalManager::finishFontInstall, this,
            &DFontMgrMainWindow::onFontInstallFinished);
}

void DFontMgrMainWindow::initShortcuts()
{
    D_D(DFontMgrMainWindow);

    //设置字体放大快捷键
    if (!m_scZoomIn) {
        m_scZoomIn = new QShortcut(this);
        m_scZoomIn->setKey(tr("Ctrl+="));
        m_scZoomIn->setContext(Qt::ApplicationShortcut);
        m_scZoomIn->setAutoRepeat(false);

        connect(m_scZoomIn, &QShortcut::activated, this, [this, d] {
            if (m_previewFontSize < MAX_FONT_SIZE)
            {
                ++m_previewFontSize;
            }
            d->fontScaleSlider->setValue(m_previewFontSize);
        });
    }

    //设置字体缩小快捷键
    if (!m_scZoomOut) {
        m_scZoomOut = new QShortcut(this);
        m_scZoomOut->setKey(tr("Ctrl+-"));
        m_scZoomOut->setContext(Qt::ApplicationShortcut);
        m_scZoomOut->setAutoRepeat(false);

        connect(m_scZoomOut, &QShortcut::activated, this, [this, d] {
            if (m_previewFontSize > MIN_FONT_SIZE)
            {
                --m_previewFontSize;
            }
            d->fontScaleSlider->setValue(m_previewFontSize);
        });
    }

    //设置字体默认大小快捷键
    if (!m_scDefaultSize) {
        m_scDefaultSize = new QShortcut(this);
        m_scDefaultSize->setKey(tr("Ctrl+0"));
        m_scDefaultSize->setContext(Qt::ApplicationShortcut);
        m_scDefaultSize->setAutoRepeat(false);

        connect(m_scDefaultSize, &QShortcut::activated, this, [this, d] {
            m_previewFontSize = DEFAULT_FONT_SIZE;
            d->fontScaleSlider->setValue(DEFAULT_FONT_SIZE);
        });
    }

    //Show shortcut --> Ctrl+Shift+/
    if (nullptr == m_scShowAllSC) {
        m_scShowAllSC = new QShortcut(this);
        m_scShowAllSC->setKey(tr("Ctrl+Shift+/"));
        m_scShowAllSC->setContext(Qt::ApplicationShortcut);
        m_scShowAllSC->setAutoRepeat(false);

        connect(m_scShowAllSC, &QShortcut::activated, this, [this] {
            this->showAllShortcut();
        });
    }

    //Show previous page --> PageUp
    if (nullptr == m_scPageUp) {
        m_scPageUp = new QShortcut(this);
        m_scPageUp->setKey(tr("PgUp"));
        m_scPageUp->setContext(Qt::ApplicationShortcut);
        m_scPageUp->setAutoRepeat(false);

        connect(m_scPageUp, &QShortcut::activated, this, [this] {;
                                                                 //For: PageUP
                                                                 //Scrolling first visible item to bottom
                                                                 QModelIndex firstVisibleItem = this->m_fontPreviewListView->indexAt(QPoint(3, 3));

                                                                 if (firstVisibleItem.isValid())
    {
        m_fontPreviewListView->scrollTo(firstVisibleItem, QAbstractItemView::PositionAtBottom);
        }
                                                                });
    }

    //Show next page --> PageDown
    if (nullptr == m_scPageDown) {
        m_scDefaultSize = new QShortcut(this);
        m_scDefaultSize->setKey(tr("PgDown"));
        m_scDefaultSize->setContext(Qt::ApplicationShortcut);
        m_scDefaultSize->setAutoRepeat(false);

        connect(m_scDefaultSize, &QShortcut::activated, this, [this] {
            //For: PageDown
            //Scrolling last visible item to top
            QRect visibleRect = m_fontPreviewListView->geometry();

            QModelIndex lastVisibleItem = this->m_fontPreviewListView->indexAt(QPoint(3, visibleRect.height() - 3));
            if (lastVisibleItem.isValid())
            {
                m_fontPreviewListView->scrollTo(lastVisibleItem, QAbstractItemView::PositionAtTop);
            }
        });
    }

    //Resize Window --> Ctrl+Alt+F
    if (nullptr == m_scWndReize) {
        m_scWndReize = new QShortcut(this);
        m_scWndReize->setKey(tr("Ctrl+Alt+F"));
        m_scWndReize->setContext(Qt::ApplicationShortcut);
        m_scWndReize->setAutoRepeat(false);

        connect(m_scWndReize, &QShortcut::activated, this, [this] {
            if (this->windowState() & Qt::WindowMaximized)
            {
                this->showNormal();
            } else if (this->windowState() == Qt::WindowNoState)
            {
                this->showMaximized();
            }
        });
    }

    //Find font --> Ctrl+F
    if (nullptr == m_scFindFont) {
        m_scFindFont = new QShortcut(this);
        m_scFindFont->setKey(tr("Ctrl+F"));
        m_scFindFont->setContext(Qt::ApplicationShortcut);
        m_scFindFont->setAutoRepeat(false);

        connect(m_scFindFont, &QShortcut::activated, this, [d] {
            d->searchFontEdit->lineEdit()->setFocus(Qt::MouseFocusReason);
        });
    }

    //Delete font --> Delete
    if (nullptr == m_scDeleteFont) {
        m_scDeleteFont = new QShortcut(this);
        m_scDeleteFont->setKey(Qt::Key_Delete);
        m_scDeleteFont->setContext(Qt::ApplicationShortcut);
        m_scDeleteFont->setAutoRepeat(false);

        connect(m_scDeleteFont, &QShortcut::activated, this, [this] {
            //Only can't delete user font
            delCurrentFont();
        });
    }

    //Add Font --> Ctrl+O
    if (nullptr == m_scAddNewFont) {
        m_scAddNewFont = new QShortcut(this);
        m_scAddNewFont->setKey(tr("Ctrl+O"));
        m_scAddNewFont->setContext(Qt::ApplicationShortcut);
        m_scAddNewFont->setAutoRepeat(false);

        connect(m_scAddNewFont, &QShortcut::activated, this, [d] {
            d->addFontButton->click();
        });
    }

    //Add favorite --> Ctrl+K
    if (nullptr == m_scAddFavFont) {
        m_scAddFavFont = new QShortcut(this);
        m_scAddFavFont->setKey(tr("Ctrl+K"));
        m_scAddFavFont->setContext(Qt::ApplicationShortcut);
        m_scAddFavFont->setAutoRepeat(false);

        connect(m_scAddFavFont, &QShortcut::activated, this, [ = ] {
            QModelIndexList itemIndexes = m_fontPreviewListView->selectedIndex(nullptr, nullptr);
            if (filterGroup != DSplitListWidget::FontGroup::CollectFont)
            {
                emit m_fontPreviewListView->onClickCollectionButton(itemIndexes, true);
            } else
            {
                emit m_fontPreviewListView->onClickCollectionButton(itemIndexes, true, true);
            }
        });
    }

    //Cancel favorite --> Ctrl+Shift+K
    if (nullptr == m_scCancelFavFont) {
        m_scCancelFavFont = new QShortcut(this);
        m_scCancelFavFont->setKey(tr("Ctrl+Shift+K"));
        m_scCancelFavFont->setContext(Qt::ApplicationShortcut);
        m_scCancelFavFont->setAutoRepeat(false);

        connect(m_scCancelFavFont, &QShortcut::activated, this, [this] {
            QModelIndexList itemIndexes = m_fontPreviewListView->selectedIndex(nullptr, nullptr);
            if (filterGroup != DSplitListWidget::FontGroup::CollectFont)
            {
                emit m_fontPreviewListView->onClickCollectionButton(itemIndexes, false);
            } else
            {
                emit m_fontPreviewListView->onClickCollectionButton(itemIndexes, false, true);
            }
//            DFontPreviewItemData currItemData = m_fontPreviewListView->currModelData();

//            if (currItemData.isCollected)
//            {
//                QAction *faveriteAction = DFontMenuManager::getInstance()->getActionByMenuAction(
//                    DFontMenuManager::M_Faverator, DFontMenuManager::MenuType::RightKeyMenu);
//                faveriteAction->trigger();
//            }
        });
    }

    //Font information --> Alt+Enter
    if (nullptr == m_scFontInfo) {
        m_scFontInfo = new QShortcut(this);
        m_scFontInfo->setKey(tr("Alt+Return"));
        m_scFontInfo->setContext(Qt::ApplicationShortcut);
        m_scFontInfo->setAutoRepeat(false);

        connect(m_scFontInfo, &QShortcut::activated, this, [this] {
            DFontPreviewItemData currItemData = m_fontPreviewListView->currModelData();

            if (!currItemData.fontInfo.filePath.isEmpty())
            {
                QAction *fontInfoAction = DFontMenuManager::getInstance()->getActionByMenuAction(
                    DFontMenuManager::M_FontInfo, DFontMenuManager::MenuType::RightKeyMenu);
                fontInfoAction->trigger();
            }
        });
    }
}

void DFontMgrMainWindow::initFontFiles()
{
    /* Bug#19657 sudo执行后，QDir::homePath路径不再是/home/user,导致新建数据库，
     * 与fc-list查出来的数据差异，导致下面的代码删除了系统字体 UT000591  */
    if (geteuid() == 0) {
        return;
    }

    m_dbManager = DFMDBManager::instance();
    DFontInfoManager *fontInfoMgr = DFontInfoManager::instance();

    QStringList allFontsList = fontInfoMgr->getAllFontPath();

    QStringList installFont = m_dbManager->getInstalledFontsPath();

    for (int i = 0; i < allFontsList.size(); ++i) {
        QString filePath = allFontsList.at(i);
        if (!m_dbManager->isSystemFont(filePath) && !installFont.contains(filePath)) {
            QFileInfo openFile(filePath);
            if (!QFile::remove(filePath))
                continue;

            QDir fileDir(openFile.path());
            if (fileDir.isEmpty()) {
                fileDir.removeRecursively();
            }
        }
    }
}

void DFontMgrMainWindow::initFontUninstallDialog()
{
    QObject::connect(m_signalManager, &SignalManager::closeUninstallDialog, this, [ = ] {
        m_needDelCount = 0;
        if (!m_isDeleting && m_isFromSys)
        {
            if (m_waitForInstall.count() > 0) {
                waitForInsert();
                m_waitForInstall.clear();
            }
            m_isFromSys = false;
        }
        m_isDeleting = false;
    }, Qt::UniqueConnection);
}

void DFontMgrMainWindow::initTileBar()
{
    D_D(DFontMgrMainWindow);

    initTileFrame();

    d->toolBarMenu = DFontMenuManager::getInstance()->createToolBarSettingsMenu();

    bool isDXcbPlatform = true;

    if (isDXcbPlatform) {
        // d->toolbar->getSettingsButton()->hide();
        titlebar()->setMenu(d->toolBarMenu);
        titlebar()->setContentsMargins(0, 0, 0, 0);

        titlebar()->setFixedHeight(FTM_TITLE_FIXED_HEIGHT);
    }
}

void DFontMgrMainWindow::initTileFrame()
{
    D_D(DFontMgrMainWindow);

    //Add logo
    titlebar()->setIcon(QIcon::fromTheme(DEEPIN_FONT_MANAGER));

    //Action area add a extra space
    d->titleActionArea = new QWidget(this);
    d->titleActionArea->setFixedSize(QSize(58, FTM_TITLE_FIXED_HEIGHT));

    QHBoxLayout *titleActionAreaLayout = new QHBoxLayout(d->titleActionArea);
    titleActionAreaLayout->setSpacing(0);
    titleActionAreaLayout->setContentsMargins(0, 0, 0, 0);

    // Add Font
    d->addFontButton = new DIconButton(DStyle::StandardPixmap::SP_IncreaseElement, this);
    d->addFontButton->setFixedSize(QSize(38, 38));
    d->addFontButton->setFlat(false);
    d->addFontButton->setFocusPolicy(Qt::FocusPolicy::NoFocus);

    titleActionAreaLayout->addWidget(d->addFontButton);

    // Search font
    d->searchFontEdit = new DSearchEdit(this);
    DFontSizeManager::instance()->bind(d->searchFontEdit, DFontSizeManager::T6);
    d->searchFontEdit->setFixedSize(QSize(FTM_SEARCH_BAR_W, FTM_SEARCH_BAR_H));
    d->searchFontEdit->setPlaceHolder(DApplication::translate("SearchBar", "Search"));

    titlebar()->addWidget(d->titleActionArea, Qt::AlignLeft | Qt::AlignVCenter);
    titlebar()->addWidget(d->searchFontEdit, Qt::AlignCenter);

    // Debug layout code
#ifdef FTM_DEBUG_LAYOUT_COLOR
    d->titleActionArea->setStyleSheet("background: red");
    d->addFontButton->setStyleSheet("background: silver");
    d->searchFontEdit->setStyleSheet("background: yellow");
#endif
}

void DFontMgrMainWindow::initMainVeiws()
{
    D_D(DFontMgrMainWindow);
    setWindowIcon(QIcon::fromTheme(DEEPIN_FONT_MANAGER));

    d->mainWndSpliter = new DSplitter(Qt::Horizontal, this);
    m_fontLoadingSpinner = new DFontSpinnerWidget(this);
    // For Debug
    // d->mainWndSpliter->setStyleSheet("QSplitter::handle { background-color: red }");

    initLeftSideBar();
    initRightFontView();

    //Disable spliter drag & resize
    QSplitterHandle *handle = d->mainWndSpliter->handle(1);
    if (handle) {
        handle->setFixedWidth(2);
        handle->setDisabled(true);

        DPalette pa = DApplicationHelper::instance()->palette(handle);
        QBrush splitBrush = pa.brush(DPalette::ItemBackground);
        pa.setBrush(DPalette::Background, splitBrush);
        handle->setPalette(pa);
        handle->setBackgroundRole(QPalette::Background);
        handle->setAutoFillBackground(true);
    }

    setCentralWidget(d->mainWndSpliter);
}

void DFontMgrMainWindow::initLeftSideBar()
{
    D_D(DFontMgrMainWindow);

    d->leftBarHolder = new QWidget(d->mainWndSpliter);
    d->leftBarHolder->setObjectName("leftMainLayoutHolder");
    d->leftBarHolder->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    d->leftBarHolder->setFixedWidth(FTM_LEFT_SIDE_BAR_WIDTH);
    d->leftBarHolder->setContentsMargins(0, 0, 2, 0);
    d->leftBarHolder->setBackgroundRole(DPalette::Base);
    d->leftBarHolder->setAutoFillBackground(true);
    // d->leftBarHolder->setAttribute(Qt::WA_TranslucentBackground, true);

    QVBoxLayout *leftMainLayout = new QVBoxLayout();
    leftMainLayout->setContentsMargins(0, 0, 0, 0);
    leftMainLayout->setSpacing(0);

    // ToDo:
    //    Need use the custom QListView replace QListWidget
    d->leftSiderBar = new DSplitListWidget(this);
    // leftSiderBar->setAttribute(Qt::WA_TranslucentBackground, true);
    d->leftSiderBar->setFrameShape(DFrame::NoFrame);
    d->leftSiderBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    leftMainLayout->addSpacing(10);
    leftMainLayout->addWidget(d->leftSiderBar);
    d->leftBarHolder->setLayout(leftMainLayout);

    d->leftSiderBar->setFocus();

    // Debug layout code
#ifdef FTM_DEBUG_LAYOUT_COLOR
    d->leftBarHolder->setStyleSheet("background: blue");
    d->leftSiderBar->setStyleSheet("background: yellow");
#endif
}

void DFontMgrMainWindow::initRightFontView()
{
    Q_D(DFontMgrMainWindow);

    // initialize state bar
    initStateBar();

    d->rightViewHolder = new QWidget(d->mainWndSpliter);
    d->rightViewHolder->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    d->rightViewHolder->setObjectName("rightMainLayoutHolder");
    d->rightViewHolder->setBackgroundRole(DPalette::Base);
    d->rightViewHolder->setAutoFillBackground(true);

    QVBoxLayout *rightMainLayout = new QVBoxLayout();
    rightMainLayout->setContentsMargins(0, 0, 0, 0);
    rightMainLayout->setSpacing(0);

    d->fontShowArea = new QWidget(this);
    //d->fontShowArea->setFrameShape(DFrame::NoFrame);
    d->fontShowArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    initFontPreviewListView(d->fontShowArea);

    // initialize statebar shadow line
    d->sbarShadowLine = new DHorizontalLine(this);
    d->sbarShadowLine->setFixedHeight(1);
    d->sbarShadowLine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    rightMainLayout->addWidget(d->fontShowArea);
    rightMainLayout->addWidget(d->sbarShadowLine);
    rightMainLayout->addWidget(d->stateBar);

    d->rightViewHolder->setLayout(rightMainLayout);

    // Debug layout code
#ifdef FTM_DEBUG_LAYOUT_COLOR
    d->fontShowArea->setStyleSheet("background: blue");
    m_fontPreviewListView->setStyleSheet("background: green");
    d->rightViewHolder->setStyleSheet("background: red");
#endif
}

//初始化字体预览ListView
void DFontMgrMainWindow::initFontPreviewListView(QWidget *parent)
{
    Q_D(DFontMgrMainWindow);

    QVBoxLayout *listViewVBoxLayout = new QVBoxLayout();
    listViewVBoxLayout->setMargin(0);
    listViewVBoxLayout->setContentsMargins(0, 0, 0, 0);
    listViewVBoxLayout->setSpacing(0);

    parent->setLayout(listViewVBoxLayout);

    m_fontPreviewListView = new DFontPreviewListView(this);
    m_fontPreviewListView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_fontPreviewListView->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
    m_fontPreviewListView->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

    m_fontPreviewListView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_fontPreviewListView->setRightContextMenu(d->rightKeyMenu);

    listViewVBoxLayout->addWidget(m_fontPreviewListView);

    // 加载图标
    DLabel *onLoadingSpinner = new DLabel(m_fontLoadingSpinner);
    onLoadingSpinner->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    onLoadingSpinner->setFixedHeight(onLoadingSpinner->fontMetrics().height());
    onLoadingSpinner->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    QVBoxLayout *lblLayoutLoad = new QVBoxLayout;
    lblLayoutLoad->addWidget(onLoadingSpinner);

    m_fontLoadingSpinner->setLayout(lblLayoutLoad);
    listViewVBoxLayout->addWidget(m_fontLoadingSpinner);

    m_fontLoadingSpinner->spinnerStart();
    m_fontPreviewListView->hide();
    d->stateBar->hide();
    m_fontLoadingSpinner->show();

    // 未搜索到结果view
    m_noResultListView = new DListView;

    DLabel *noResultLabel = new DLabel(m_noResultListView);
    noResultLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    noResultLabel->setFixedHeight(noResultLabel->fontMetrics().height() + 30);
    noResultLabel->setText(DApplication::translate("SearchBar", "No search results"));

    QFont labelFont = noResultLabel->font();
    labelFont.setWeight(QFont::ExtraLight);
    noResultLabel->setFont(labelFont);
    noResultLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    DFontSizeManager::instance()->bind(noResultLabel, DFontSizeManager::T4);

    QVBoxLayout *lblLayout = new QVBoxLayout;
    lblLayout->addWidget(noResultLabel);

    m_noResultListView->setLayout(lblLayout);
    listViewVBoxLayout->addWidget(m_noResultListView);

    m_noResultListView->hide();

    // 未安装字体view
    m_noInstallListView = new DListView;

    DLabel *noInstallLabel = new DLabel(m_noInstallListView);
    noInstallLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    noInstallLabel->setFixedHeight(noInstallLabel->fontMetrics().height() + 30);
    noInstallLabel->setText(DApplication::translate("SearchBar", "No fonts"));

    QFont labelFontNoInstall = noInstallLabel->font();
    labelFontNoInstall.setWeight(QFont::ExtraLight);
    noInstallLabel->setFont(labelFontNoInstall);
    noInstallLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    DFontSizeManager::instance()->bind(noInstallLabel, DFontSizeManager::T4);

    QVBoxLayout *lblLayoutNoInstall = new QVBoxLayout;
    lblLayoutNoInstall->addWidget(noInstallLabel);

    m_noInstallListView->setLayout(lblLayoutNoInstall);
    listViewVBoxLayout->addWidget(m_noInstallListView);

    m_noInstallListView->hide();
}

void DFontMgrMainWindow::initStateBar()
{
    Q_D(DFontMgrMainWindow);

    QHBoxLayout *stateBarLayout = new QHBoxLayout();
    stateBarLayout->setContentsMargins(0, 0, 0, 0);
    stateBarLayout->setSpacing(0);

    d->stateBar = new QWidget(this);
    //d->stateBar->setFrameShape(DFrame::NoFrame);
    d->stateBar->setFixedHeight(FTM_SBAR_HEIGHT);
    d->stateBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    d->textInputEdit = new DLineEdit(this);
//    QFont searchFont;
//    searchFont.setPixelSize(14);
//    d->textInputEdit->setFont(searchFont);
    //d->textInputEdit->setMinimumSize(QSize(FTM_SBAR_TXT_EDIT_W,FTM_SBAR_TXT_EDIT_H));
    DFontSizeManager::instance()->bind(d->textInputEdit, DFontSizeManager::T6);
    d->textInputEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    d->textInputEdit->setClearButtonEnabled(true);
    d->textInputEdit->lineEdit()->setPlaceholderText(DApplication::translate("StateBar", "Input preview text"));

    d->fontScaleSlider = new DSlider(Qt::Orientation::Horizontal, this);
    d->fontScaleSlider->setFixedSize(FTM_SBAR_SLIDER_W, FTM_SBAR_SLIDER_H);
    // d->fontScaleSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//    d->fontScaleSlider->setTracking(true);
//    d->fontScaleSlider->setTickPosition(QSlider::NoTicks);
//    d->fontScaleSlider->setRange(MIN_FONT_SIZE, MAX_FONT_SIZE);
    d->fontScaleSlider->setMinimum(MIN_FONT_SIZE);
    d->fontScaleSlider->setMaximum(MAX_FONT_SIZE);
    //设置初始显示字体大小
    d->fontScaleSlider->setValue(DEFAULT_FONT_SIZE);

    d->fontSizeLabel = new DLabel(this);
    QFont fontScaleFont;
    fontScaleFont.setPixelSize(14);
    d->fontSizeLabel->setFont(fontScaleFont);
    d->fontSizeLabel->setFixedSize(FTM_SBAR_FSIZE_LABEL_W, FTM_SBAR_FSIZE_LABEL_H);
    d->fontSizeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

//    QFont fontSize;
//    fontSize.setPixelSize(14);
//    d->fontSizeLabel->setFont(fontSize);
    DFontSizeManager::instance()->bind(d->fontSizeLabel, DFontSizeManager::T6);
    // d->fontSizeLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    // Init the default font size
    QString defaultFontSize;
    defaultFontSize.sprintf(FMT_FONT_SIZE, DEFAULT_FONT_SIZE);
    d->fontSizeLabel->setText(defaultFontSize);

    stateBarLayout->addSpacing(10);
    stateBarLayout->addWidget(d->textInputEdit, 1);
    stateBarLayout->addSpacing(20);
    stateBarLayout->addWidget(d->fontScaleSlider);
    stateBarLayout->addSpacing(20);
    stateBarLayout->addWidget(d->fontSizeLabel);
    stateBarLayout->addSpacing(20);

    d->stateBar->setLayout(stateBarLayout);

    // Debug layout code
#ifdef FTM_DEBUG_LAYOUT_COLOR
    d->stateBar->setStyleSheet("background: green");
    d->textInputEdit->setStyleSheet("background: blue");
    d->fontScaleSlider->setStyleSheet("background: yellow");
    d->fontSizeLabel->setStyleSheet("background: yellow");
#endif
}

void DFontMgrMainWindow::handleAddFontEvent()
{
    Q_D(DFontMgrMainWindow);

    DFileDialog dialog;
    dialog.setFileMode(DFileDialog::ExistingFiles);
    dialog.setNameFilter(Utils::suffixList());

    QString historyDir = d->settingsQsPtr->value("dir").toString();
    if (historyDir.isEmpty()) {
        historyDir = QDir::homePath();
    }
    dialog.setDirectory(historyDir);

    const int mode = dialog.exec();

    // save the directory string to config file.
    d->settingsQsPtr->setValue("dir", dialog.directoryUrl().toLocalFile());

    // if click cancel button or close button.
    if (mode != QDialog::Accepted) {
        return;
    }

    Q_EMIT fileSelected(dialog.selectedFiles());
}

void DFontMgrMainWindow::handleMenuEvent(QAction *action)
{
    if (action->data().isValid()) {
        bool ok = false;
        int type = action->data().toInt(&ok);

        if (ok) {
            DFontMenuManager::MenuAction actionId = static_cast<DFontMenuManager::MenuAction>(type);

            // Add menu handler code here
            switch (actionId) {
            case DFontMenuManager::MenuAction::M_AddFont: {
                handleAddFontEvent();
            }
            break;
            case DFontMenuManager::MenuAction::M_FontInfo: {
                DFontPreviewItemData currItemData = m_fontPreviewListView->currModelData();
                DFontInfoDialog *fontInfoDlg = new DFontInfoDialog(&currItemData, this);
                fontInfoDlg->exec();
            }
            break;
            case DFontMenuManager::MenuAction::M_DeleteFont: {
                delCurrentFont();
            }
            break;
            case DFontMenuManager::MenuAction::M_ExportFont: {
                exportFont();
            }
            break;
            case DFontMenuManager::MenuAction::M_EnableOrDisable: {
                DFontPreviewItemData currItemData = m_fontPreviewListView->currModelData();
                QModelIndexList itemIndexes = m_fontPreviewListView->selectedIndex(nullptr, nullptr);
                if (filterGroup != DSplitListWidget::FontGroup::ActiveFont) {
                    emit m_fontPreviewListView->onClickEnableButton(itemIndexes, !currItemData.isEnabled);
                } else {
                    emit m_fontPreviewListView->onClickEnableButton(itemIndexes, !currItemData.isEnabled, true);
                }

//                  emit m_fontPreviewListView->onClickEnableButton(currItemData, !currItemData.isEnabled);
            }
            break;
            case DFontMenuManager::MenuAction::M_Faverator: {
//                QModelIndex modelIndex = m_fontPreviewListView->currModelIndex();
                DFontPreviewItemData currItemData = m_fontPreviewListView->currModelData();
                QModelIndexList itemIndexes = m_fontPreviewListView->selectedIndex(nullptr, nullptr);

                if (filterGroup != DSplitListWidget::FontGroup::CollectFont) {
                    emit m_fontPreviewListView->onClickCollectionButton(itemIndexes, !currItemData.isCollected);
                } else {
                    emit m_fontPreviewListView->onClickCollectionButton(itemIndexes, !currItemData.isCollected, true);
                }
            }
            break;
            case DFontMenuManager::MenuAction::M_ShowFontPostion:
                showFontFilePostion();
                break;
            default:
                qDebug() << "handleMenuEvent->(id=" << actionId << ")";
                break;
            }
        }
    }
}

void DFontMgrMainWindow::installFont(const QStringList &files)
{
    qDebug() << __FUNCTION__ << files;

    if (m_fIsInstalling) {
        qDebug() << "Already exist a installtion flow";
        return;
    }

    m_dfNormalInstalldlg = new DFInstallNormalWindow(files, this);

    if (m_isQuickMode) {
        m_dfNormalInstalldlg->setSkipException(true);
    }



    //Set installtion flag
    /*
     * Add font from + ,menu, drag file to main view
     * to task bar can start a installtion flow, so must
     * to set flag avoid
     */
    m_fIsInstalling = true;

//    Dtk::Widget::moveToCenter(m_dfNormalInstalldlg);
    m_dfNormalInstalldlg->exec();

    //Clear installtion flag when NormalInstalltion window is closed
    m_fIsInstalling = false;
}

void DFontMgrMainWindow::installFontFromSys(const QStringList &files)
{
    this->m_isFromSys = true;

    if (m_isDeleting) {
        qDebug() << "Is deleting ,quit";
        m_waitForInstall = files;
        return;
    } else {
        installFont(files);
    }

}

void DFontMgrMainWindow::initRightKeyMenu()
{
    Q_D(DFontMgrMainWindow);

    d->rightKeyMenu = DFontMenuManager::getInstance()->createRightKeyMenu();
}

void DFontMgrMainWindow::setQuickInstallMode(bool isQuick)
{
#ifdef QT_QML_DEBUG
    qDebug() << __FUNCTION__ << " isQuickMode=" << isQuick;
#endif
    m_isQuickMode = isQuick;
}

void DFontMgrMainWindow::hideQucikInstallWindow()
{
    if (m_quickInstallWnd.get() != nullptr) {
        m_quickInstallWnd->setVisible(false);
    }
}

void DFontMgrMainWindow::InitQuickWindowIfNeeded()
{
    if (m_quickInstallWnd.get() == nullptr) {
        m_quickInstallWnd.reset(new DFQuickInstallWindow());

        // Quick install mode handle
        QObject::connect(this, &DFontMgrMainWindow::quickModeInstall, this,
        [this](const QStringList & files) {
            connect(m_quickInstallWnd.get(), &DFQuickInstallWindow::quickInstall, this,
            [this, files]() {
                this->installFont(files);
            });
            m_quickInstallWnd.get()->setWindowModality(Qt::WindowModal);
            m_quickInstallWnd->onFileSelected(files);
            m_quickInstallWnd->show();
            m_quickInstallWnd->raise();       //Reative the window
            m_quickInstallWnd->activateWindow();

            Dtk::Widget::moveToCenter(m_quickInstallWnd.get());
        });
    }
}

void DFontMgrMainWindow::forceNoramlInstalltionQuitIfNeeded()
{
    if (m_fIsInstalling) {
        qDebug() << "In normal installtion flow, force quit!";
        m_dfNormalInstalldlg->breakInstalltion();
    }
}

void DFontMgrMainWindow::setDeleteFinish()
{
    m_fIsDeleting = false;
}

void DFontMgrMainWindow::onSearchTextChanged(const QString &currStr)
{
    Q_D(DFontMgrMainWindow);

    if (!m_fontPreviewListView->isListDataLoadFinished()) {
        return;
    }

//    QString strSearchFontName = currStr;
    const QString strSearchFontName = currStr;
    qDebug() << strSearchFontName << endl;

    m_searchTextStatusIsEmpty = d->searchFontEdit->text().isEmpty();

    DFontPreviewProxyModel *filterModel = m_fontPreviewListView->getFontPreviewProxyModel();

    //根据搜索框内容实时过滤列表
    filterModel->setFilterKeyColumn(0);
    filterModel->setFilterFontNamePattern(strSearchFontName);
    filterModel->setEditStatus(m_searchTextStatusIsEmpty);

    qDebug() << __FUNCTION__ << "filter Count:" << filterModel->rowCount() << endl;

//    QString previewText = d->textInputEdit->text();
    const QString previewText = d->textInputEdit->text();
    onPreviewTextChanged(previewText);
    m_fontPreviewListView->scrollToTop();
}

void DFontMgrMainWindow::onPreviewTextChanged(const QString &currStr)
{
    Q_D(DFontMgrMainWindow);

    if (!m_fontPreviewListView->isListDataLoadFinished()) {
        return;
    }

    QString previewText = currStr;
    if (0 == currStr.length()) {
        previewText = QString(DApplication::translate("Font", "Don't let your dreams be dreams"));
    }

    QString strFontSize = d->fontSizeLabel->text();
    int iFontSize = strFontSize.remove("px").toInt();

    DFontPreviewProxyModel *filterModel = m_fontPreviewListView->getFontPreviewProxyModel();
    qDebug() << __FUNCTION__ << "filter Count:" << filterModel->rowCount() << endl;

    for (int rowIndex = 0; rowIndex < filterModel->rowCount(); rowIndex++) {
        QModelIndex modelIndex = filterModel->index(rowIndex, 0);
        filterModel->setData(modelIndex, QVariant(previewText), Dtk::UserRole + 1);
        filterModel->setData(modelIndex, QVariant(iFontSize), Dtk::UserRole + 2);
        filterModel->setEditStatus(m_searchTextStatusIsEmpty);
    }
}

void DFontMgrMainWindow::onFontSizeChanged(int fontSize)
{
    Q_EMIT m_signalManager->refreshCurRect();
    if (!m_fontPreviewListView->isListDataLoadFinished()) {
        return;
    }

    DFontPreviewProxyModel *filterModel = m_fontPreviewListView->getFontPreviewProxyModel();
    qDebug() << __FUNCTION__ << "filter Count:" << filterModel->rowCount() << endl;

    for (int rowIndex = 0; rowIndex < filterModel->rowCount(); rowIndex++) {
        QModelIndex modelIndex = filterModel->index(rowIndex, 0);
        filterModel->setData(modelIndex, QVariant(fontSize), Dtk::UserRole + 2);
        filterModel->setEditStatus(m_searchTextStatusIsEmpty);
    }
    Q_EMIT m_signalManager->prevFontChanged();
}

void DFontMgrMainWindow::showFontFilePostion()
{
    DFontPreviewItemData currItemData = m_fontPreviewListView->currModelData();

    if (-1 != currItemData.strFontId && currItemData.fontInfo.filePath.length() > 0) {
        DDesktopServices::showFileItem(currItemData.fontInfo.filePath);
    }
}

void DFontMgrMainWindow::onLeftSiderBarItemClicked(int index)
{
    Q_D(DFontMgrMainWindow);

    if (!m_fontPreviewListView->isListDataLoadFinished()) {
        //save index to update
        m_leftIndex = index;
        return;
    }

    m_leftIndex = 0;

    qDebug() << __FUNCTION__ << index << endl;
    filterGroup = qvariant_cast<DSplitListWidget::FontGroup>(index);
    emit m_signalManager->currentFontGroup(filterGroup);

    qDebug() << filterGroup << endl;

    DFontPreviewProxyModel *filterModel = m_fontPreviewListView->getFontPreviewProxyModel();
    m_fontPreviewListView->clearSelection();
    filterModel->setFilterKeyColumn(0);
    filterModel->setFilterGroup(filterGroup);
    filterModel->setEditStatus(m_searchTextStatusIsEmpty);

    QString previewText = d->textInputEdit->text();
    onPreviewTextChanged(previewText);
}

void DFontMgrMainWindow::onFontInstallFinished(const QStringList &fileList)
{
    Q_D(DFontMgrMainWindow);

    Q_EMIT m_fontPreviewListView->requestAdded(fileList);
    d->textInputEdit->textChanged(d->textInputEdit->text());
    if (!fileList.isEmpty()) {
        showInstalledFiles(fileList);
    }
}

void DFontMgrMainWindow::onFontUninstallFinished(const QStringList &uninstallIndex)
{
//    qDebug() << "finished remove fonts:" << uninstallIndex << endl;
//    for (QString filePath : uninstallIndex) {
//        m_fontPreviewListView->deleteFontModelIndex(filePath);
//    }
//    Q_EMIT requestDeleted(uninstallIndex);
}



/* 判断FontListView的结果并显示对应状态
 * dShow = 0 :查找到信息，显示正常
 * dShow = 1 :未查到信息，显示“无搜索结果”
 * dShow = 2 :未安装字体，显示“暂无字体”
 * default   :默认有信息，显示正常
 */
void DFontMgrMainWindow::onFontListViewRowCountChanged(unsigned int bShow)
{
    Q_D(DFontMgrMainWindow);
    bool isSpinnerHidden = m_fontLoadingSpinner->isHidden();
    switch (bShow) {
    case 0:
        while (isSpinnerHidden) {
            m_fontPreviewListView->show();
            m_noResultListView->hide();
            d->stateBar->show();
            if (m_noInstallListView->isVisible()) {
                m_noInstallListView->hide();
            }
            break;
        }
        return;
    case 1:
        while (isSpinnerHidden) {
            m_fontPreviewListView->hide();
            QTimer::singleShot(5, [ = ]() {
                m_noResultListView->show();
            });
            d->stateBar->hide();
            if (m_noInstallListView->isVisible()) {
                m_noInstallListView->hide();
            }
            break;
        }
        return;
    case 2:
        while (isSpinnerHidden) {
            m_fontPreviewListView->hide();
            d->stateBar->hide();
            if (m_noResultListView->isVisible()) {
                m_noResultListView->hide();
            }
            d->leftSiderBar->setFocus();
            m_noInstallListView->show();
            break;
        }
        return;
    default:
        m_fontPreviewListView->show();
        m_noResultListView->hide();
        d->stateBar->show();
        break;
    }
}

// 0 正在加载
// 1 完成加载
void DFontMgrMainWindow::onLoadStatus(int type)
{
    if (0 == type || 1 == type) {
        switch (type) {
        case 0:
            m_fontPreviewListView->hide();
            if (m_noResultListView->isVisible()) {
                m_noResultListView->hide();
            }
            m_fontLoadingSpinner->spinnerStop();
            m_fontLoadingSpinner->spinnerStart();
            m_fontLoadingSpinner->show();
            break;
        case 1:
            if (m_leftIndex >= 0) {
                onLeftSiderBarItemClicked(m_leftIndex);
            }
            m_fontLoadingSpinner->hide();
            m_fontLoadingSpinner->spinnerStop();
            m_fontPreviewListView->show();
            break;
        default:
            break;
        }
    }
    if (type == 1 && !m_fileList.isEmpty()) {
        showInstalledFiles(m_fileList);
    }
}

void DFontMgrMainWindow::onShowMessage(int successCount)
{
    QString messageA;
    QString messageB;

//    if (successCount == 1 && systemFontCount != 0) {
//        messageA = DApplication::translate("Font", "%1 font installed").arg(successCount);
//        DMessageManager::instance()->sendMessage(this, QIcon(":/images/ok.svg"), messageA);
//        messageB = DApplication::translate("Font", "The other %2 system fonts have already been installed").arg(systemFontCount);
//        DMessageManager::instance()->sendMessage(this, QIcon(":/images/exception-logo.svg"), messageB);
//    }
//    if (successCount != 1) {
//        messageA = DApplication::translate("Font", "%1 fonts installed").arg(successCount);
//        DMessageManager::instance()->sendMessage(this, QIcon(":/images/ok.svg"), messageA);
//        if (systemFontCount > 0) {
//            messageB = DApplication::translate("Font", "The other %2 system fonts have already been installed").arg(systemFontCount);
//            DMessageManager::instance()->sendMessage(this, QIcon(":/images/exception-logo.svg"), messageB);
//        }
//    }

    if (successCount == 1) {
        messageA = DApplication::translate("DFontMgrMainWindow", "%1 font installed").arg(successCount);
        DMessageManager::instance()->sendMessage(this, QIcon(":/images/ok.svg"), messageA);

    } else if (successCount > 1) {
        messageA = DApplication::translate("DFontMgrMainWindow", "%1 fonts installed").arg(successCount);
        DMessageManager::instance()->sendMessage(this, QIcon(":/images/ok.svg"), messageA);
    }
}

void DFontMgrMainWindow::delCurrentFont()
{
    qDebug() << __FUNCTION__ << m_fIsDeleting;
    if (m_fIsDeleting)
        return;
    m_fIsDeleting = true;
    int deleteCnt = 0;
    int systemCnt = 0;
    QStringList uninstallFilePath = m_fontPreviewListView->selectedFonts(&deleteCnt, &systemCnt);
    if (deleteCnt < 1) {
        m_fIsDeleting = false;
        return;
    }

    m_isDeleting = true;
    DFDeleteDialog *confirmDelDlg = new DFDeleteDialog(this, deleteCnt, systemCnt, this);

    /* Bug#19111 恢复标记位，不用考虑用户操作 UT00591 */
    connect(confirmDelDlg, &DFDeleteDialog::closed, this, [this]() {
        m_isDeleting = false;
    });

//    confirmDelDlg->move((this->width() - confirmDelDlg->width() - 230 + mapToGlobal(QPoint(0, 0)).x()), (mapToGlobal(QPoint(0, 0)).y() + 180));
    confirmDelDlg->move(this->geometry().center() - confirmDelDlg->rect().center());
    confirmDelDlg->exec();
}

void DFontMgrMainWindow::exportFont()
{
    int cnt = 0;
    int systemCnt = 0;
    int count = 0;
    QStringList files = m_fontPreviewListView->selectedFonts(&cnt, &systemCnt);
    if (cnt < 1)
        return;
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/" + tr("Fonts") + "/";
    QDir dir(desktopPath);
    if (!dir.exists())
        dir.mkpath(desktopPath);
    for (QString file : files) {
        QFile::copy(file, desktopPath + QFileInfo(file).fileName());
    }
    QString message;
    if (files.size() == 1) {
        message = DApplication::translate("Main", "The font exported to your desktop");
        DMessageManager::instance()->sendMessage(this, QIcon(":/images/ok.svg"), message);
    } else {
        message = DApplication::translate("Main", "%1 fonts exported to your desktop").arg(files.size());
        DMessageManager::instance()->sendMessage(this, QIcon(":/images/ok.svg"), message);
    }
}

void DFontMgrMainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    //Q_D(DFontMgrMainWindow);

    if (event->mimeData()->hasUrls()) {
        QList<QUrl> dragFiles = event->mimeData()->urls();

        qDebug() << dragFiles;
        if (dragFiles.size() == 1) {
            //For one-drag check MIME,ignore non-font file
            if (Utils::isFontMimeType(dragFiles[0].path())) {
                event->accept();
                return;
            }
        } else {
            //Multi-drag just accept all file at start
            //will filter non-font files in drapEvent
            event->accept();
            return;
        }
    }
    qDebug() << __FUNCTION__ << "ignore";
    event->ignore();
}

void DFontMgrMainWindow::dropEvent(QDropEvent *event)
{

    if (event->mimeData()->hasUrls()) {

        QStringList installFileList;

        QList<QUrl> dragFiles = event->mimeData()->urls();

        if (dragFiles.size() > 1) {
            foreach (auto it, event->mimeData()->urls()) {
                if (Utils::isFontMimeType(it.path())) {
                    installFileList.append(it.path());
                }
            }
        } else {
            if (Utils::isFontMimeType(dragFiles[0].path())) {
                installFileList.append(dragFiles[0].path());
            }
        }

        //Check if need to trigger installtion
        if (installFileList.size() > 0) {
            event->accept();
            Q_EMIT fileSelected(installFileList);
        } else {
            event->ignore();
        }
    } else {
        event->ignore();
    }
}

void DFontMgrMainWindow::resizeEvent(QResizeEvent *event)
{
    if (0 == int(QWidget::windowState())) {
        m_winHight = geometry().height();
        m_winWidth = geometry().width();
    }
}

void DFontMgrMainWindow::showAllShortcut()
{
    QRect rect = window()->geometry();
    QPoint pos(rect.x() + rect.width() / 2,
               rect.y() + rect.height() / 2);

    QJsonObject shortcutObj;
    QJsonArray jsonGroups;

    QMap<QString, QString> shortcutKeymap = {
        {DApplication::translate("Shortcut", "Help"), "F1"},
//        {"Zoom in",            "Ctrl+-"},
//        {"Zoom out",           "Ctrl++"},
//        {"Reset font",         "Ctrl+0"},
//        {"Close window",       "Alt+F4"},
        {DApplication::translate("Shortcut", "Display shortcuts"),  "Ctrl+Shift+?"},
        {DApplication::translate("Shortcut", "Page up"), "PageUp"},
        {DApplication::translate("Shortcut", "Page down"), "PageDown"},
//        {"Resize window",      "Ctrl+Alt+F"},
//        {"Find",               "Ctrl+F"},
        {DApplication::translate("Shortcut", "Delete"), "Delete"},
        {DApplication::translate("Shortcut", "Add font"), "Ctrl+O"},
        {DApplication::translate("Shortcut", "Favorite"), "Ctrl+K"},
        {DApplication::translate("Shortcut", "Unfavorite"), "Ctrl+Shift+K"},
        {DApplication::translate("Shortcut", "Font info"), "Alt+Enter"},
    };

    QJsonObject fontMgrJsonGroup;
    fontMgrJsonGroup.insert("groupName", DApplication::translate("Main", "Font Manager"));
    QJsonArray fontJsonItems;

    for (QMap<QString, QString>::iterator it = shortcutKeymap.begin();
            it != shortcutKeymap.end(); it++) {
        QJsonObject jsonItem;
        jsonItem.insert("name", DApplication::translate("Shortcuts", it.key().toUtf8()));
        jsonItem.insert("value", it.value().replace("Meta", "Super"));
        fontJsonItems.append(jsonItem);
    }

    fontMgrJsonGroup.insert("groupItems", fontJsonItems);
    jsonGroups.append(fontMgrJsonGroup);

    shortcutObj.insert("shortcut", jsonGroups);

    QJsonDocument doc(shortcutObj);

    QStringList shortcutString;
    QString param1 = "-j=" + QString(doc.toJson().data());
    QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
    shortcutString << param1 << param2;

    QProcess *shortcutViewProcess = new QProcess();
    shortcutViewProcess->startDetached("deepin-shortcut-viewer", shortcutString);

    connect(shortcutViewProcess, SIGNAL(finished(int)), shortcutViewProcess, SLOT(deleteLater()));
}

void DFontMgrMainWindow::showInstalledFiles(const QStringList &fileList)
{
    D_D(DFontMgrMainWindow);

    d->leftSiderBar->setCurrentIndex(d->leftSiderBar->model()->index(DSplitListWidget::UserFont, 0));
    onLeftSiderBarItemClicked(DSplitListWidget::UserFont);
}

void DFontMgrMainWindow::checkCloseUninstallDialog()
{
    if (m_deledCount == m_needDelCount) {
        m_deledCount = 0;
        m_needDelCount = 0;
        emit SignalManager::instance()->closeUninstallDialog();
    }
}

void DFontMgrMainWindow::waitForInsert()
{

    m_dfNormalInstalldlg = new DFInstallNormalWindow(m_waitForInstall, this);
    if (m_isQuickMode) {
        m_dfNormalInstalldlg->setSkipException(true);
    }



    //Set installtion flag
    /*
     * Add font from + ,menu, drag file to main view
     * to task bar can start a installtion flow, so must
     * to set flag avoid
     */
    m_fIsInstalling = true;

    Dtk::Widget::moveToCenter(m_dfNormalInstalldlg);
    m_dfNormalInstalldlg->exec();

    //Clear installtion flag when NormalInstalltion window is closed
    m_fIsInstalling = false;
}

//弹出删除进度框后执行删除操作
void DFontMgrMainWindow::startToDelete()
{
    m_uninstallFilePath.clear();
    m_uninstallFilePath = m_fontPreviewListView->selectedFonts(nullptr, nullptr);
//    qDebug() << m_uninstallFilePath << endl;
    m_needDelCount = m_uninstallFilePath.count();
    DFontPreviewItemData currItemData = m_fontPreviewListView->currModelData();
    qDebug() << "Confirm delete:" << currItemData.fontInfo.filePath
             << " is system font:" << currItemData.fontInfo.isSystemFont;
    //force delete all fonts
    Q_EMIT requestDeleted(m_uninstallFilePath);
    m_fontManager->setType(DFontManager::UnInstall);
    m_fontManager->setUnInstallFile(m_uninstallFilePath);
    m_fontManager->start();
}

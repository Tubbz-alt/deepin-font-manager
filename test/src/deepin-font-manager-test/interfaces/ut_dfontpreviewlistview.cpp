#include "dfontpreviewlistview.h"
#include "dfontinfomanager.h"
#include "dfontpreviewitemdelegate.h"
#include "globaldef.h"
#include "dfmxmlwrapper.h"
#include "views/dfontmgrmainwindow.h"

#include <DLog>
#include <DMenu>
#include <DGuiApplicationHelper>
#include <DApplication>
#include <DMessageManager>

#include <QSignalSpy>
#include <QFontDatabase>
#include <QSet>
#include <QScroller>
#include <QTest>

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"

namespace {
class TestDFontPreviewListView : public testing::Test
{

protected:
    void SetUp()
    {
        listview = new DFontPreviewListView();
    }
    void TearDown()
    {

    }
    QString filepath = "/home/zhaogongqiang/Desktop/1048字体/ArkanaScriptRough.otf";

    DFontPreviewListView *listview;
};

//参数化测试
class TestUpdateCurrentFontGroup : public::testing::TestWithParam<int>
{
public:
    DFontMgrMainWindow *mw = new DFontMgrMainWindow;
    DFontPreviewListView *listview = new DFontPreviewListView();
};

QWidget *stub_viewport()
{
    QWidget *widget = new QWidget;
    widget->setFixedHeight(120);
    return widget;
}

bool stub_False()
{
    return false;
}

bool stub_True()
{
    return true;
}

void  stub_Return()
{
    return ;
}

QPoint stub_pos()
{
    return QPoint(626, 22) ;
}

QList<DFontPreviewItemData> stub_getFontModelList()
{
    QList<DFontPreviewItemData> list;

    DFontPreviewItemData data;
    data.fontInfo.filePath = "first";

    list << data;
    return list;
}

DFontPreviewItemData stub_getFontData()
{
    DFontPreviewItemData data;
    data.fontInfo.filePath = "first";

    return data;
}

DFontPreviewItemData stub_getFontDataPath()
{
    DFontPreviewItemData data;
    data.fontInfo.filePath = "/usr/share/fonts/opentype/noto/NotoSansCJK-Bold.ttc";

    return data;
}

}

TEST_F(TestDFontPreviewListView, checkInitFontListData)
{
    QSignalSpy spy(listview, SIGNAL(onLoadFontsStatus(int)));
    listview->initFontListData();
    EXPECT_TRUE(spy.count() == 1) << spy.count() << "checkInitFontListData++";
}

TEST_F(TestDFontPreviewListView, checkOnItemRemoved)
{
    DFontPreviewItemData data;
    data.fontInfo.filePath = filepath;
    data.appFontId = 1;
    listview->onItemRemoved(data);
}

TEST_F(TestDFontPreviewListView, checkOnItemRemovedSys)
{
    DFontPreviewItemData data;
    data.fontInfo.filePath = filepath;
    data.appFontId = 1;
//    listview->onItemRemovedFromSys(data);
}

//mark
//TEST_P(TestUpdateCurrentFontGroup, checkUpadteCurrentFontGroup)
//{
//    int n =  GetParam();

//    listview->updateCurrentFontGroup(n);

//    EXPECT_TRUE(n == listview->m_currentFontGroup) << n << "+++++++++++++++";

//}

INSTANTIATE_TEST_CASE_P(HandleTrueReturn, TestUpdateCurrentFontGroup, testing::Values(0, 1, 2, 3, 4, 5, 6));


TEST_F(TestDFontPreviewListView, checkCount)
{
    qDebug() << listview->model()->insertRow(0);

    listview->count();
    EXPECT_TRUE(1 == listview->count());
}

TEST_F(TestDFontPreviewListView, checkCancelDel)
{
    listview->cancelDel();
    EXPECT_TRUE(listview->m_selectAfterDel == -1);
}

TEST_F(TestDFontPreviewListView, checkViewChanged)
{
    listview->viewChanged();

    EXPECT_TRUE(listview->m_selectAfterDel == -1);
}

//markPositionBeforeRemoved
TEST_F(TestDFontPreviewListView, checkMarkPositionBeforeRemoved_isdelete)
{
    listview->model()->insertRow(0);
    QModelIndex index = listview->model()->index(0, 0);
    listview->selectionModel()->select(index, QItemSelectionModel::Select);
    listview->markPositionBeforeRemoved();
    EXPECT_TRUE(listview->m_selectAfterDel == -2);
}

//TEST_F(TestDFontPreviewListView, checkMarkPositionBeforeRemoved_notdelete)
//{
//    listview->model()->insertRow(0);
//    QModelIndex index = listview->model()->index(0, 0);
//    QModelIndexList list;
//    list << index;
//    listview->markPositionBeforeRemoved();
//    EXPECT_TRUE(listview->m_selectAfterDel == 0);
//    qDebug() << listview->isVisible() << endl;

//}

//TEST_F(TestDFontPreviewListView, checkGetOnePageCount)
//{
//    int count = listview->getOnePageCount();
//    EXPECT_TRUE(count == 12);

//    listview->model()->insertRow(0);

////    QAbstractScrollArea
//    Stub s;
//    s.set(ADDR(QAbstractScrollArea, viewport), stub_viewport);


//    count = listview->getOnePageCount();
//    EXPECT_TRUE(count == 1);
//}

TEST_F(TestDFontPreviewListView, checkAppendFilePath)
{
    QStringList list;
    list << "aaa" << "bbb";
    listview->appendFilePath(&list, "ccc");
    EXPECT_TRUE(list.count() == 3);

    listview->appendFilePath(&list, "aaa");
    EXPECT_TRUE(list.count() == 3);
}

//TEST_F(TestDFontPreviewListView, checkGetCurFontStrName)
//{
//    DFontPreviewItemData d;
//    d.fontData.strFontName = "aaa";

//    listview->m_curFontData = d;

//    EXPECT_TRUE(listview->getCurFontData().fontData.strFontName == "aaa");
//    EXPECT_TRUE(listview->getCurFontStrName() == "aaa");
//}

//TEST_F(TestDFontPreviewListView, checkGetFontData)
//{
//    FontData data;
//    data.strFontName = "A";
//    EXPECT_TRUE(listview->getFontData(data).appFontId == -1);
//}

TEST_F(TestDFontPreviewListView, checkSetRecoveryTabFocusState)
{
    listview->setRecoveryTabFocusState(false);
    EXPECT_FALSE(listview->m_recoveryTabFocusState);
}

TEST_F(TestDFontPreviewListView, checkSetIsTabFocus)
{
    listview->setIsTabFocus(false);
    EXPECT_FALSE(listview->getIsTabFocus());
}

TEST_F(TestDFontPreviewListView, checkUpdateSpinner)
{
    QSignalSpy s(listview, SIGNAL(requestShowSpinner(bool, bool, DFontSpinnerWidget::SpinnerStyles)));
    listview->m_curTm = QDateTime::currentMSecsSinceEpoch();
    usleep(360000);
    DFontSpinnerWidget::SpinnerStyles style = DFontSpinnerWidget::SpinnerStyles::StartupLoad;
    listview->updateSpinner(style, true);
    EXPECT_TRUE(s.count() == 1) << s.count();
}

TEST_F(TestDFontPreviewListView, checkUpdateModel)
{

    QSignalSpy s1(listview, SIGNAL(requestShowSpinner(bool, bool, DFontSpinnerWidget::SpinnerStyles)));
    QSignalSpy s2(listview, SIGNAL(rowCountChanged()));
    QSignalSpy s3(listview, SIGNAL(deleteFinished()));
    QSignalSpy s4(listview->m_signalManager, SIGNAL(fontSizeRequestToSlider()));

    listview->updateModel(false);

    EXPECT_TRUE(s1.count() == 1) << s1.count();
    EXPECT_TRUE(s2.count() == 1) << s2.count();
    EXPECT_TRUE(s3.count() == 1) << s3.count();
    EXPECT_TRUE(s4.count() == 1) << s4.count();
}

//TEST_F(TestDFontPreviewListView, checkGetCollectionIconRect)
//{
//    QPoint p(0, 0);
//    QPoint q(60, 80);
//    QRect c(p, q);
////    qDebug() << c.right() << "++++++++++" << c.top() << endl;
////    QRect r = listview->getCollectionIconRect(c);
////    EXPECT_TRUE(r.height() == 32);
//}

//TEST_F(TestDFontPreviewListView, checkGetCheckboxRect)
//{
//    QPoint p(0, 0);
//    QPoint q(60, 80);
//    QRect c(p, q);
////    qDebug() << c.right() << "++++++++++" << c.top() << endl;
////    QRect r = listview->getCheckboxRect(c);
////    EXPECT_TRUE(r.height() == 30) << r.height();
//}

TEST_F(TestDFontPreviewListView, checkOnUpdateCurrentFont)
{
    listview->m_fontChanged = true;
    listview->onUpdateCurrentFont();
    EXPECT_FALSE(listview->m_fontChangeTimer->isActive());

}
// sortModelIndexList
//TEST_F(TestDFontPreviewListView, checkSortModelIndexList)
//{
//    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
//    m_fontPreviewItemModel->setColumnCount(1);
//    DFontPreviewProxyModel *fpm = new DFontPreviewProxyModel;
//    fpm->setSourceModel(m_fontPreviewItemModel);
//    DFontPreviewListView *listview = new DFontPreviewListView;
//    listview->setModel(fpm);
//    m_fontPreviewItemModel->insertRows(0, 5);

//    QModelIndexList list;

//    QModelIndex index = m_fontPreviewItemModel->index(0, 0);
//    QModelIndex index1 = m_fontPreviewItemModel->index(1, 0);

//    list << index << index1;

//    listview->sortModelIndexList(list);
//    EXPECT_TRUE(list.first().row() == 1);
//}

//deleteFontModelIndex
//TEST_F(TestDFontPreviewListView, checkDeleteFontModelIndex)
//{
////    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
////    m_fontPreviewItemModel->setColumnCount(1);
////    DFontPreviewProxyModel *fpm = new DFontPreviewProxyModel;
////    fpm->setSourceModel(m_fontPreviewItemModel);
////    DFontPreviewListView *listview = new DFontPreviewListView;
////    listview->setModel(fpm);
//    listview->m_fontPreviewItemModel->insertRows(1, 5);
//    listview->m_fontPreviewProxyModel->insertRow(1);
//    qDebug() << listview->m_fontPreviewItemModel->rowCount();

////    listview->deleteFontModelIndex("", true);
//}


////UpdateSelection maek
//TEST_F(TestDFontPreviewListView, checkUpdateSelection)
//{
//    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
//    m_fontPreviewItemModel->setColumnCount(1);
//    DFontPreviewProxyModel *fpm = new DFontPreviewProxyModel;
//    fpm->setSourceModel(m_fontPreviewItemModel);

//    listview->setModel(fpm);
////    m_fontPreviewItemModel->insertRows(0, 5);
//    listview->m_fontPreviewProxyModel->insertRows(0, 5);

//    listview->m_selectAfterDel = 1;

//    listview->updateSelection();

//    listview->m_selectAfterDel = 5;
//    listview->updateSelection();

//    Stub s;
//    s.set(ADDR(DFontPreviewListView, isAtListviewTop), stub_False);

//    Stub s2;
//    s2.set(ADDR(DFontPreviewListView, isAtListviewBottom), stub_True);
//    listview->updateSelection();

//}

//TEST_F(TestDFontPreviewListView, refreshRect)
//{
//    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
//    m_fontPreviewItemModel->setColumnCount(1);
//    DFontPreviewProxyModel *fpm = new DFontPreviewProxyModel;
//    fpm->setSourceModel(m_fontPreviewItemModel);
//    DFontPreviewListView *listview = new DFontPreviewListView;
//    listview->setModel(fpm);
//    m_fontPreviewItemModel->insertRows(0, 5);

//    listview->refreshRect();

//    listview->selectAll();

//    listview->refreshRect();
//}

TEST_F(TestDFontPreviewListView, checkScrollWithTheSelected)
{
    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
    m_fontPreviewItemModel->setColumnCount(1);
    DFontPreviewProxyModel *fpm = new DFontPreviewProxyModel;
    fpm->setSourceModel(m_fontPreviewItemModel);
    DFontPreviewListView *listview = new DFontPreviewListView;
    listview->setModel(fpm);
    m_fontPreviewItemModel->insertRows(0, 5);
    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    listview->scrollWithTheSelected();
    EXPECT_TRUE(listview->m_currentSelectedRow == 0);

    listview->selectAll();
    listview->scrollWithTheSelected();
}

TEST_F(TestDFontPreviewListView, checkChangeFontFileForce)
{
    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
    m_fontPreviewItemModel->setColumnCount(1);
    DFontPreviewProxyModel *fpm = new DFontPreviewProxyModel;
    fpm->setSourceModel(m_fontPreviewItemModel);
    DFontPreviewListView *listview = new DFontPreviewListView;
    listview->setModel(fpm);
    m_fontPreviewItemModel->insertRows(0, 5);
    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    listview->changeFontFile("", true);
}

//TEST_F(TestDFontPreviewListView, checkChangeFontFileUnForce)
//{
//    Stub s;
//    s.set(ADDR(QFileInfo, isDir), stub_True);

//    Stub s1;
//    s1.set(ADDR(DFontPreviewListDataThread, getFontModelList), stub_getFontModelList);


//    listview->changeFontFile("first", false);

//}

//TEST_F(TestDFontPreviewListView, checkDeleteCurFonts)
//{
//    QSignalSpy spy(listview, SIGNAL(requestUpdateModel(int, bool)));

//    QStringList list;

//    listview->deleteCurFonts(list, false);
//    qDebug() << spy.count();
//    EXPECT_TRUE(spy.count() == 1);

//    Stub s1;
//    s1.set(ADDR(DFontPreviewListDataThread, getFontModelList), stub_getFontModelList);

//    list << "first";

//    listview->deleteCurFonts(list, true);
//    qDebug() << spy.count() << endl;
//    EXPECT_TRUE(spy.count() == 2);

//}

TEST_F(TestDFontPreviewListView, checkUpdateChangedDir)
{
    QSignalSpy spy(listview, SIGNAL(itemRemoved(const DFontPreviewItemData &)));
    QSignalSpy spy2(listview, SIGNAL(rowCountChanged()));

//    Stub s1;
//    s1.set(ADDR(DFontPreviewListDataThread, getFontModelList), stub_getFontModelList);

    QList<DFontPreviewItemData> list;

    DFontPreviewItemData data;
    data.fontInfo.filePath = "first";

    list << data;

    listview->m_dataThread->m_fontModelList.clear();
    listview->m_dataThread->m_fontModelList = list;

    listview->updateChangedDir();

    qDebug() << spy.count() << "!!!!!!!!!!" << spy2.count() << endl;
    EXPECT_TRUE(spy.count() == 1);
    EXPECT_TRUE(spy2.count() == 1);
}

TEST_F(TestDFontPreviewListView, checkUpdateChangedFile)
{
    Stub s1;
    s1.set(ADDR(DFontPreviewListView, changeFontFile), stub_Return);


    QSignalSpy spy(listview, SIGNAL(rowCountChanged()));

    listview->updateChangedFile("first");

    EXPECT_TRUE(spy.count() == 1);

}

TEST_F(TestDFontPreviewListView, checkClearHoverState)
{
    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
    m_fontPreviewItemModel->setColumnCount(1);

    DFontPreviewProxyModel *fpm = new DFontPreviewProxyModel;
    fpm->setSourceModel(m_fontPreviewItemModel);

    DFontPreviewListView *listview = new DFontPreviewListView;
    listview->setModel(fpm);

    m_fontPreviewItemModel->insertRows(0, 5);
    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    listview->m_hoverModelIndex = listview->m_fontPreviewProxyModel->index(2, 0);

    FontData data;
    data.setHoverState(IconHover);

    listview->m_fontPreviewProxyModel->setData(listview->m_fontPreviewProxyModel->index(2, 0), QVariant::fromValue(data), Qt::DisplayRole);

    listview->clearHoverState();
    EXPECT_TRUE(listview->m_hoverModelIndex == QModelIndex());

    FontData itemData =
        qvariant_cast<FontData>(listview->m_fontPreviewProxyModel->data(listview->m_fontPreviewProxyModel->index(2, 0)));

    EXPECT_TRUE(itemData.getHoverState() == IconNormal);
}

TEST_F(TestDFontPreviewListView, checkClearPressStateMoveClear)
{
    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
    m_fontPreviewItemModel->setColumnCount(1);

    DFontPreviewProxyModel *fpm = new DFontPreviewProxyModel;
    fpm->setSourceModel(m_fontPreviewItemModel);

    DFontPreviewListView *listview = new DFontPreviewListView;
    listview->setModel(fpm);

    m_fontPreviewItemModel->insertRows(0, 5);
    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    listview->m_previousPressPos = 1;

    FontData data;
    data.setHoverState(IconHover);

    QModelIndex index = listview->m_fontPreviewProxyModel->index(1, 0);
    listview->m_fontPreviewProxyModel->setData(index, QVariant::fromValue(data));

    QModelIndex index2 = listview->m_fontPreviewProxyModel->index(2, 0);
    listview->m_pressModelIndex = index2;
    listview->m_fontPreviewProxyModel->setData(index2, QVariant::fromValue(data));

    listview->clearPressState(DFontPreviewListView::MoveClear, 0);
    EXPECT_TRUE(listview->m_pressModelIndex == QModelIndex());
    FontData itemData =
        qvariant_cast<FontData>(listview->m_fontPreviewProxyModel->data(listview->m_pressModelIndex));
    EXPECT_TRUE(itemData.getHoverState() == IconNormal);
}

TEST_F(TestDFontPreviewListView, checkClearPressStatePreviousClear)
{

    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    listview->m_previousPressPos = 1;

    FontData data;
    data.setHoverState(IconHover);

    QModelIndex index = listview->m_fontPreviewProxyModel->index(1, 0);
    listview->m_fontPreviewProxyModel->setData(index, QVariant::fromValue(data));

    listview->clearPressState(DFontPreviewListView::PreviousClear, 0);

    FontData itemData =
        qvariant_cast<FontData>(listview->m_fontPreviewProxyModel->data(index));
    EXPECT_TRUE(itemData.getHoverState() == IconNormal);

}

TEST_F(TestDFontPreviewListView, checkCurrModelData)
{
    Stub s;
    s.set(ADDR(DFontPreviewListDataThread, getFontData), stub_getFontData);

    listview->m_fontPreviewProxyModel->insertRows(0, 5);
    listview->m_previousPressPos = 1;

    FontData data;
    data.strFontName = "first";

    QModelIndex index = listview->m_fontPreviewProxyModel->index(1, 0);
    listview->m_fontPreviewProxyModel->setData(index, QVariant::fromValue(data));
    listview->selectionModel()->select(index, QItemSelectionModel::Select);

    DFontPreviewItemData data2 = listview->currModelData();
    EXPECT_TRUE(data2.fontInfo.filePath == "first");
}

TEST_F(TestDFontPreviewListView, checkCurrModelIndex)
{
    Stub s;
    s.set(ADDR(DFontPreviewListDataThread, getFontData), stub_getFontData);

    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    QModelIndex index = listview->m_fontPreviewProxyModel->index(0, 0);
    listview->selectionModel()->select(index, QItemSelectionModel::Select);
    index = listview->m_fontPreviewProxyModel->index(1, 0);
    listview->selectionModel()->select(index, QItemSelectionModel::Select);

    QModelIndex currentIndex = listview->currModelIndex();

    EXPECT_TRUE(currentIndex.row() == 0);

}

TEST_F(TestDFontPreviewListView, checkOnCollectBtnClicked)
{
    QSignalSpy spy(listview, SIGNAL(rowCountChanged()));

    QModelIndexList list;

    listview->onCollectBtnClicked(list, true, true);
    qDebug() << spy.count();
    EXPECT_TRUE(spy.count() == 0);

    Stub s;
    s.set(ADDR(DFontPreviewListDataThread, updateItemStatus), stub_Return);
//    DFontPreviewListView *listview = new DFontPreviewListView;
    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    FontData fdata;

    fdata.strFontName = "first";
    fdata.fontState = 0x01;

    listview->selectAll();

    listview->m_fontPreviewProxyModel->setData(listview->m_fontPreviewProxyModel->index(1, 0), QVariant::fromValue(fdata), Qt::DisplayRole);

    list.clear();
    list << listview->m_fontPreviewProxyModel->index(1, 0);

    listview->onCollectBtnClicked(list, true, true);

    QModelIndex index = listview->m_fontPreviewProxyModel->index(1, 0);

    FontData itemData =
        qvariant_cast<FontData>(listview->m_fontPreviewProxyModel->data(index));
//mark
    qDebug() << itemData.fontState << endl;
    EXPECT_TRUE(itemData.fontState == 0x03);

    qDebug() << spy.count();
    EXPECT_TRUE(spy.count() == 1);
}

TEST_F(TestDFontPreviewListView, checkOnEnableBtnClickedEnable)
{
    QSignalSpy spy(listview, SIGNAL(rowCountChanged()));

    Stub s;
    s.set(ADDR(DFontPreviewListDataThread, getFontData), stub_getFontDataPath);

    QModelIndexList list;
    listview->onEnableBtnClicked(list, 0, 0, false, true);
    EXPECT_TRUE(spy.count() == 0);

    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    Stub s1;
    s1.set(ADDR(DFontPreviewListDataThread, updateItemStatus), stub_Return);

    FontData fdata;
    fdata.strFontName = "first";
    fdata.fontState = 0x01;

    listview->selectAll();

    listview->m_fontPreviewProxyModel->setData(listview->m_fontPreviewProxyModel->index(1, 0), QVariant::fromValue(fdata), Qt::DisplayRole);

    list.clear();

    QModelIndex index = listview->m_fontPreviewItemModel->index(1, 0);
    list << index;
    listview->onEnableBtnClicked(list, 0, 1, true, true);

    FontData itemData =
        qvariant_cast<FontData>(listview->m_fontPreviewProxyModel->data(index));
    qDebug() << itemData.fontState << endl;
    EXPECT_TRUE(itemData.fontState == 0x00);

    DFontMgrMainWindow *mw = new DFontMgrMainWindow;
    listview->setParent(mw);
    listview->m_parentWidget = mw;
    listview->onEnableBtnClicked(list, 1, 1, false, true);
    listview->onEnableBtnClicked(list, 0, 0, false, true);

}

//TEST_F(TestDFontPreviewListView, checkHoverState)
//{
//    Stub s1;
//    s1.set(ADDR(QWidget, isVisible), stub_True);

//    listview->m_fontPreviewProxyModel->insertRows(0, 5);

//    listview->selectAll();
//    listview->onRightMenuShortCutActivated();
//}

//mark
//TEST_F(TestDFontPreviewListView, checkIsAtListviewTopTrue)
//{
//    listview->verticalScrollBar()->setMinimum(100);
//    listview->verticalScrollBar()->setValue(100);
//    EXPECT_TRUE(listview->isAtListviewTop());
//}

//mark
//TEST_F(TestDFontPreviewListView, checkIsAtListviewTopFalse)
//{
//    listview->verticalScrollBar()->setMinimum(100);
//    listview->verticalScrollBar()->setMaximum(800);
//    listview->verticalScrollBar()->setValue(800);
//    qDebug() << listview->verticalScrollBar()->minimum() << "++++++++" << listview->verticalScrollBar()->value() << endl;

//    EXPECT_FALSE(listview->isAtListviewTop());
//}

//mark
//TEST_F(TestDFontPreviewListView, checkIsAtListviewBottomTrue)
//{
//    listview->verticalScrollBar()->setMinimum(100);
//    listview->verticalScrollBar()->setMaximum(800);
//    listview->verticalScrollBar()->setValue(800);
//    EXPECT_TRUE(listview->isAtListviewBottom());
//}

//TEST_F(TestDFontPreviewListView, checkIsAtListviewBottomFalse)
//{
//    listview->verticalScrollBar()->setMinimum(100);
//    listview->verticalScrollBar()->setMaximum(800);
//    listview->verticalScrollBar()->setValue(100);
//    EXPECT_TRUE(listview->isAtListviewTop());
//}

TEST_F(TestDFontPreviewListView, checkUpdateShiftSelect)
{
    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    listview->m_currentSelectedRow = 1;

    QModelIndex index = listview->m_fontPreviewProxyModel->index(1, 0);
    listview->updateShiftSelect(index);

    EXPECT_TRUE(listview->selectedIndexes().count() == 1);

    index = listview->m_fontPreviewProxyModel->index(2, 0);
    listview->updateShiftSelect(index);
    EXPECT_TRUE(listview->selectedIndexes().count() == 2);

    listview->m_currentSelectedRow = -1;
    index = listview->m_fontPreviewProxyModel->index(3, 0);
    listview->updateShiftSelect(index);
    EXPECT_TRUE(listview->selectedIndexes().count() == 4);
}

TEST_F(TestDFontPreviewListView, checkDisableFonts)
{
    Stub s1;
    s1.set(ADDR(DFMXmlWrapper, createFontConfigFile), stub_True);
    listview->disableFont("first");
    listview->disableFonts();
    EXPECT_TRUE(listview->m_disableFontList.count() == 0);

    s1.set(ADDR(DFMXmlWrapper, createFontConfigFile), stub_False);

    listview->disableFont("first");
    listview->disableFonts();
    EXPECT_TRUE(listview->m_disableFontList.count() == 1);
}

TEST_F(TestDFontPreviewListView, checkEnableFonts)
{
    Stub s1;
    s1.set(ADDR(DFMXmlWrapper, createFontConfigFile), stub_True);
    listview->enableFont("first");
    listview->enableFonts();
    EXPECT_TRUE(listview->m_enableFontList.count() == 0);

    s1.set(ADDR(DFMXmlWrapper, createFontConfigFile), stub_False);

    listview->enableFont("first");
    listview->enableFonts();
    EXPECT_TRUE(listview->m_enableFontList.count() == 1);
}

TEST_F(TestDFontPreviewListView, checkEventFilterFocusOut)
{
    QSignalSpy spy(listview->m_signalManager, SIGNAL(setLostFocusState(bool)));

    QEvent *e = new QEvent(QEvent::FocusOut);

    listview->m_isLostFocusOfLeftKey = true;
    listview->eventFilter(listview, e);
    EXPECT_FALSE(listview->m_IsTabFocus);
}

TEST_F(TestDFontPreviewListView, checkEventFilterFocusIn)
{
    QSignalSpy spy(listview->m_signalManager, SIGNAL(requestSetTabFocusToAddBtn()));

    QFocusEvent *e = new QFocusEvent(QEvent::FocusIn, Qt::TabFocusReason);

    listview->m_isGetFocusFromSlider = true;

    listview->eventFilter(listview, e);
    EXPECT_TRUE(spy.count() == 1);
}

TEST_F(TestDFontPreviewListView, checkKeyPressEventFilterEmpty)
{
    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    listview->keyPressEventFilter(QModelIndexList(), true, false, false);
    EXPECT_TRUE(listview->selectedIndexes().count() == 1);
    EXPECT_TRUE(listview->selectedIndexes().first().row() == 4);

    listview->keyPressEventFilter(QModelIndexList(), false, true, false);
    EXPECT_TRUE(listview->selectedIndexes().count() == 1);
    EXPECT_TRUE(listview->selectedIndexes().first().row() == 0);
}

TEST_F(TestDFontPreviewListView, checkKeyPressEventFilterUp)
{
    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    QModelIndex index = listview->m_fontPreviewProxyModel->index(1, 0);
    QModelIndexList indexList;
    indexList.append(index);

    listview->m_currentSelectedRow = 0;
    listview->keyPressEventFilter(indexList, true, false, true);
    EXPECT_TRUE(listview->selectedIndexes().count() == 1);
    EXPECT_TRUE(listview->selectedIndexes().first().row() == 0);

    listview->m_currentSelectedRow = 1;
    listview->keyPressEventFilter(indexList, true, false, true);
    EXPECT_TRUE(listview->selectedIndexes().count() == 1);
    EXPECT_TRUE(listview->selectedIndexes().first().row() == 0);

    indexList.clear();
    index = listview->m_fontPreviewProxyModel->index(0, 0);
    indexList.append(index);
    listview->keyPressEventFilter(indexList, true, false, false);

//    Stub s1;
//    s1.set(ADDR(DFontPreviewListView, isAtListviewTop), stub_True);
//    listview->keyPressEventFilter(indexList, true, false, false);
//    EXPECT_TRUE(listview->m_currentSelectedRow = 4);
//    EXPECT_TRUE(listview->selectedIndexes().first().row() == 4);

    listview->m_currentSelectedRow = 4;
    indexList.clear();
    index = listview->m_fontPreviewProxyModel->index(3, 0);
    indexList.append(index);
    listview->keyPressEventFilter(indexList, true, false, false);
    EXPECT_TRUE(listview->selectedIndexes().first().row() == 3);
    EXPECT_TRUE(listview->m_currentSelectedRow == 3);
}

TEST_F(TestDFontPreviewListView, checkKeyPressEventFilterDown)
{
    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    QModelIndex index = listview->m_fontPreviewProxyModel->index(1, 0);
    QModelIndexList indexList;
    indexList.append(index);

    listview->m_currentSelectedRow = 2;
    listview->keyPressEventFilter(indexList, false, true, true);
    EXPECT_TRUE(listview->selectedIndexes().count() == 1);
    EXPECT_TRUE(listview->selectedIndexes().first().row() == 2);

    index = listview->m_fontPreviewProxyModel->index(3, 0);
    indexList.clear();
    indexList.append(index);
    listview->keyPressEventFilter(indexList, false, true, true);
    EXPECT_TRUE(listview->selectedIndexes().count() == 2);
    EXPECT_TRUE(listview->selectedIndexes().at(1).row() == 4);

    listview->keyPressEventFilter(indexList, false, true, false);
    EXPECT_TRUE(listview->selectedIndexes().count() == 1);
    EXPECT_TRUE(listview->selectedIndexes().first().row() == 3);

}

TEST_F(TestDFontPreviewListView, checkkeyPressEventKey_End)
{
    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    QTest::keyPress(listview->viewport(), Qt::Key_End, Qt::NoModifier);
    EXPECT_TRUE(listview->currentIndex().row() == 4);

    listview->m_currentSelectedRow = 0;
    QTest::keyPress(listview->viewport(), Qt::Key_End, Qt::ShiftModifier);
    EXPECT_TRUE(listview->selectedIndexes().count() == 5);

    listview->m_currentSelectedRow = -1;
    QTest::keyPress(listview->viewport(), Qt::Key_End, Qt::ShiftModifier);
    EXPECT_TRUE(listview->m_currentSelectedRow == 0);

}

TEST_F(TestDFontPreviewListView, checkkeyPressEventKey_Home)
{
    listview->m_fontPreviewProxyModel->insertRows(0, 5);

    QTest::keyPress(listview->viewport(), Qt::Key_Home, Qt::NoModifier);
    EXPECT_TRUE(listview->currentIndex().row() == 0);

    listview->m_currentSelectedRow = 4;
    QTest::keyPress(listview->viewport(), Qt::Key_Home, Qt::ShiftModifier);
    EXPECT_TRUE(listview->selectedIndexes().count() == 5);
}

TEST_F(TestDFontPreviewListView, checkkeyPressEventKey_UpDown)
{
    listview->m_fontPreviewProxyModel->insertRows(0, 5);
    listview->selectAll();

    QSignalSpy spy(listview->m_signalManager, SIGNAL(requestSetLeftSiderBarFocus()));


    QTest::keyPress(listview->viewport(), Qt::Key_Up, Qt::ShiftModifier);

    QTest::keyPress(listview->viewport(), Qt::Key_Down, Qt::ShiftModifier);

    QTest::keyPress(listview->viewport(), Qt::Key_Down, Qt::ControlModifier);

    QTest::keyPress(listview->viewport(), Qt::Key_Up, Qt::NoModifier);

    QTest::keyPress(listview->viewport(), Qt::Key_Down, Qt::NoModifier);

    QTest::keyPress(listview->viewport(), Qt::Key_Left, Qt::NoModifier);

    EXPECT_TRUE(spy.count() == 1);
}

TEST_F(TestDFontPreviewListView, checkrowsAboutToBeRemoved)
{
    listview->m_fontPreviewProxyModel->insertRows(0, 5);
    listview->selectAll();

    QModelIndex index = listview->m_fontPreviewProxyModel->index(1, 0);

    listview->rowsAboutToBeRemoved(index, 1, 1);

    EXPECT_TRUE(listview->selectionModel()->currentIndex().row() == 1);
}

TEST_F(TestDFontPreviewListView, checkIfHasSelection)
{
    listview->m_fontPreviewProxyModel->insertRows(0, 5);
    listview->checkIfHasSelection();
    EXPECT_TRUE(listview->m_currentSelectedRow == -1);
}

//TEST_F(TestDFontPreviewListView, checkOnMouseLeftBtnReleased)
//{
////    Stub s1;
////    s1.set((bool(QRect::*)(const QPoint &, bool)) ADDR(QRect, contains), stub_show);

////    Stub stub;
////        stub.set((bool(QRect::*)(const QPoint &,bool))ADDR(QRect,contains), foo_stub_int);
//}









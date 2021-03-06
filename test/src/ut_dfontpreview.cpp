#include "dfontpreviewer.h"

#include "utils.h"

#include <QPainter>
#include <QPaintEvent>
#include <QRect>

#include <DPalette>
#include <DApplicationHelper>

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"

namespace {
class TestDFontPreviewer : public testing::Test
{

protected:
    void SetUp()
    {
        fm = new DFontPreviewer();
    }
    void TearDown()
    {
        delete fm;
    }
    // Some expensive resource shared by all tests.
    QWidget *w = new QWidget;
    DFontPreviewer *fm;
};

}

TEST_F(TestDFontPreviewer, checkInitData)
{
    fm->InitData();
    EXPECT_FALSE(fm->m_previewTexts.isEmpty());
    EXPECT_TRUE(fm->m_previewTexts.contains("汉体书写信息技术标准相容"));
}

TEST_F(TestDFontPreviewer, checkOnPreviewFontChanged)
{
    fm->onPreviewFontChanged();
    EXPECT_FALSE(fm->m_previewTexts.isEmpty());

    fm->setPreviewFontPath("/home/zhaogongqiang/Desktop/1048字体/ArkanaScriptRough.otf");
    fm->onPreviewFontChanged();
    EXPECT_FALSE(fm->m_previewTexts.isEmpty());
    //    EXPECT_FALSE();
}

TEST_F(TestDFontPreviewer, checkPaintevent)
{
    QRect r;
    QPaintEvent *p = new QPaintEvent(r);
    fm->paintEvent(p);
}











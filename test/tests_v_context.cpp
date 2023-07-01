#include <gtest/gtest.h>

#include "midas.hpp"
// vctx.V.set_label("a2_V");

TEST(TestsVContext, Basic)
{
    auto vctx = midas::v_context("test", midas::dimension::DIM2);
    vctx.get_x().set_bins(10, 0, 10).set_label("a2_x").set_unit("mm");
    vctx.get_y().set_bins(10, 0, 10).set_label("a2_y");
    vctx.get_v().set_label("a2_V");
    vctx.format_diff_axis();

    ASSERT_EQ(TString(" [mm]"), vctx.get_x().format_unit());
    ASSERT_EQ(TString("d^{2}/da2_xda2_y"), vctx.get_label());

    vctx = midas::v_context("test", midas::dimension::DIM3);

    vctx.get_x().set_bins(10, 0, 10).set_label("a2_x").set_unit("mm");
    vctx.get_y().set_bins(10, 0, 10).set_label("a2_y");
    vctx.get_z().set_bins(10, 0, 10).set_label("a2_z");
    vctx.get_v().set_label("a2_V");
    vctx.format_diff_axis();

    ASSERT_EQ(TString("d^{3}/da2_xda2_yda2_z"), vctx.get_label());

    midas::v_context vctx_2 = vctx;
    vctx_2.format_diff_axis();

    ASSERT_EQ(TString("d^{3}/da2_xda2_yda2_z"), vctx_2.get_label());

    ASSERT_TRUE(vctx == vctx_2);

    vctx_2.get_x().set_bins(13, 0, 10);
    ASSERT_FALSE(vctx == vctx_2);

    ASSERT_TRUE(vctx == vctx);
}

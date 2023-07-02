#include <gtest/gtest.h>

#include "midas.hpp"

TEST(TestsContext, CreationByDim)
{
    auto ctx1 = midas::basic_context("test", midas::dimension::DIM1);
    ctx1.get_x();
    ASSERT_THROW(ctx1.get_y(), midas::dimension_error);
    ASSERT_THROW(ctx1.get_z(), midas::dimension_error);

    auto ctx2 = midas::basic_context("test", midas::dimension::DIM2);
    ctx2.get_x();
    ctx2.get_y();
    ASSERT_THROW(ctx2.get_z(), midas::dimension_error);

    auto ctx3 = midas::basic_context("test", midas::dimension::DIM3);
    ctx3.get_x();
    ctx3.get_y();
    ctx3.get_z();
}

TEST(TestsContext, CreationByAxes)
{
    midas::axis_config x;
    midas::axis_config y;
    midas::axis_config z;

    auto ctx1 = midas::basic_context("test", x);
    ctx1.get_x();
    ASSERT_THROW(ctx1.get_y(), midas::dimension_error);
    ASSERT_THROW(ctx1.get_z(), midas::dimension_error);

    auto ctx2 = midas::basic_context("test", x, y);
    ctx2.get_x();
    ctx2.get_y();
    ASSERT_THROW(ctx2.get_z(), midas::dimension_error);

    auto ctx3 = midas::basic_context("test", x, y, z);
    ctx3.get_x();
    ctx3.get_y();
    ctx3.get_z();
}

TEST(TestsContext, Updating)
{
    auto ctx = midas::basic_context("test", midas::dimension::DIM3);
    ctx.get_x().set_label("d2_x").set_unit("mm");
    ctx.get_y().set_label("d2_y").set_unit("");
    ctx.get_z().set_label("d2_z").set_unit("cm");

    ASSERT_EQ(TString(" [mm]"), ctx.get_x().format_unit());
    ASSERT_EQ(TString(""), ctx.get_y().format_unit());
    ASSERT_EQ(TString(" [cm]"), ctx.get_z().format_unit());

    midas::basic_context ctx_2 = ctx;

    ASSERT_EQ(TString(" [cm]"), ctx_2.get_z().format_unit());

    ASSERT_TRUE(ctx == ctx_2);

    ctx_2.get_x().set_bins(13, 0, 0);
    ASSERT_FALSE(ctx == ctx_2);

    ASSERT_TRUE(ctx == ctx);
}

TEST(TestsContext, Extend)
{
    auto ctx = midas::basic_context("test", midas::dimension::NODIM);
    ASSERT_THROW(ctx.get_x(), midas::dimension_error);
    ASSERT_THROW(ctx.get_y(), midas::dimension_error);
    ASSERT_THROW(ctx.get_z(), midas::dimension_error);

    ctx.extend();
    ASSERT_NO_THROW(ctx.get_x());
    ASSERT_THROW(ctx.get_y(), midas::dimension_error);
    ASSERT_THROW(ctx.get_z(), midas::dimension_error);

    ctx.extend();
    ASSERT_NO_THROW(ctx.get_x());
    ASSERT_NO_THROW(ctx.get_y());
    ASSERT_THROW(ctx.get_z(), midas::dimension_error);

    ctx.extend();
    ASSERT_NO_THROW(ctx.get_x());
    ASSERT_NO_THROW(ctx.get_y());
    ASSERT_NO_THROW(ctx.get_z());

    ASSERT_THROW(ctx.extend(), midas::dimension_error);
}

TEST(TestsContext, Reduce)
{
    auto ctx = midas::basic_context("test", midas::dimension::DIM3);
    ASSERT_NO_THROW(ctx.get_x());
    ASSERT_NO_THROW(ctx.get_y());
    ASSERT_NO_THROW(ctx.get_z());

    ctx.reduce();
    ASSERT_NO_THROW(ctx.get_x());
    ASSERT_NO_THROW(ctx.get_y());
    ASSERT_THROW(ctx.get_z(), midas::dimension_error);

    ctx.reduce();
    ASSERT_NO_THROW(ctx.get_x());
    ASSERT_THROW(ctx.get_y(), midas::dimension_error);
    ASSERT_THROW(ctx.get_z(), midas::dimension_error);

    ASSERT_THROW(ctx.reduce(), midas::dimension_error);

    auto ctx2 = midas::basic_context("test", midas::dimension::NODIM);
    ASSERT_THROW(ctx2.reduce(), midas::dimension_error);
}

TEST(TestsContext, ReduceAndExtend)
{
    midas::axis_config x;
    x.set_bins(10, -10, 10);

    midas::axis_config y;
    y.set_bins(20, -20, 20);

    midas::axis_config z;
    z.set_bins(30, -30, 30);

    auto ctx = midas::basic_context("test", x, y, z);
    ctx.reduce(); // to DIM2
    ctx.reduce(); // to DIM1

    ctx.extend(); // to DIM2
    // After extension, the new dimensions should be cleared
    ASSERT_EQ(ctx.get_y().get_bins(), 0);
    ASSERT_EQ(ctx.get_y().get_min(), 0.0f);
    ASSERT_EQ(ctx.get_y().get_max(), 0.0f);

    ctx.extend(); // to DIM3
    // After extension, the new dimensions should be cleared
    ASSERT_EQ(ctx.get_z().get_bins(), 0);
    ASSERT_EQ(ctx.get_z().get_min(), 0.0f);
    ASSERT_EQ(ctx.get_z().get_max(), 0.0f);
}

TEST(TestsContext, Comapre)
{
    midas::axis_config x;
    x.set_bins(10, -10, 10);

    midas::axis_config y;
    y.set_bins(20, -20, 20);

    midas::axis_config z;
    z.set_bins(30, -30, 30);

    auto ctx1 = midas::basic_context("test", x);
    auto ctx1_1 = midas::basic_context("test", x);
    auto ctx1_2 = midas::basic_context("test", y);

    auto ctx2 = midas::basic_context("test", x, y);
    auto ctx2_1 = midas::basic_context("test", x, y);
    auto ctx2_2 = midas::basic_context("test", y, z);

    auto ctx3 = midas::basic_context("test", x, y, z);
    auto ctx3_1 = midas::basic_context("test", x, y, z);
    auto ctx3_2 = midas::basic_context("test", y, z, x);

    ASSERT_TRUE(ctx1 == ctx1);
    ASSERT_TRUE(ctx1 == ctx1_1);
    ASSERT_FALSE(ctx1 != ctx1_1);
    ASSERT_FALSE(ctx1 == ctx1_2);
    ASSERT_TRUE(ctx1 != ctx1_2);
    ASSERT_FALSE(ctx1 == ctx2);
    ASSERT_FALSE(ctx1 == ctx3);

    ASSERT_TRUE(ctx2 == ctx2);
    ASSERT_TRUE(ctx2 == ctx2_1);
    ASSERT_FALSE(ctx2 != ctx2_1);
    ASSERT_FALSE(ctx2 == ctx2_2);
    ASSERT_TRUE(ctx2 != ctx2_2);
    ASSERT_FALSE(ctx2 == ctx3);
    ASSERT_FALSE(ctx2 == ctx1);

    ASSERT_TRUE(ctx3 == ctx3);
    ASSERT_TRUE(ctx3 == ctx3_1);
    ASSERT_FALSE(ctx3 != ctx3_1);
    ASSERT_FALSE(ctx3 == ctx3_2);
    ASSERT_TRUE(ctx3 != ctx3_2);
    ASSERT_FALSE(ctx3 == ctx1);
    ASSERT_FALSE(ctx3 == ctx2);

    auto ctx0 = midas::basic_context("test", midas::dimension::NODIM);
    ASSERT_FALSE(ctx0 == ctx1);
    ASSERT_TRUE(ctx0 != ctx1);
    ASSERT_FALSE(ctx0 == ctx2);
    ASSERT_TRUE(ctx0 != ctx2);
    ASSERT_FALSE(ctx0 == ctx3);
    ASSERT_TRUE(ctx0 != ctx3);
}

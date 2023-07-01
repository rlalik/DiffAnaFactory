#include <gtest/gtest.h>

#include "midas.hpp"

TEST(TestsContext, CreationDim1)
{
    auto ctx = midas::context("test", midas::dimension::DIM1);
    ctx.get_x().set_label("d2_x").set_unit("mm");
    ASSERT_THROW(ctx.get_y(), midas::dimension_error);
    ASSERT_THROW(ctx.get_z(), midas::dimension_error);
}

TEST(TestsContext, CreationDim2)
{
    auto ctx = midas::context("test", midas::dimension::DIM2);
    ctx.get_x().set_label("d2_x").set_unit("mm");
    ctx.get_y().set_label("d2_y").set_unit("");
    ASSERT_THROW(ctx.get_z(), midas::dimension_error);
}

TEST(TestsContext, CreationDim3)
{
    auto ctx = midas::context("test", midas::dimension::DIM3);
    ctx.get_x().set_label("d2_x").set_unit("mm");
    ctx.get_y().set_label("d2_y").set_unit("");
    ctx.get_z().set_label("d2_z").set_unit("cm");
}

TEST(TestsContext, Creation)
{
    auto ctx = midas::context("test", midas::dimension::DIM3);
    ctx.get_x().set_label("d2_x").set_unit("mm");
    ctx.get_y().set_label("d2_y").set_unit("");
    ctx.get_z().set_label("d2_z").set_unit("cm");

    ASSERT_EQ(TString(" [mm]"), ctx.get_x().format_unit());
    ASSERT_EQ(TString(""), ctx.get_y().format_unit());
    ASSERT_EQ(TString(" [cm]"), ctx.get_z().format_unit());

    midas::context ctx_2 = ctx;

    ASSERT_EQ(TString(" [cm]"), ctx_2.get_z().format_unit());

    ASSERT_TRUE(ctx == ctx_2);

    ctx_2.get_x().set_bins(13, 0, 0);
    ASSERT_FALSE(ctx == ctx_2);

    ASSERT_TRUE(ctx == ctx);
}

TEST(TestsContext, Extend)
{
    auto ctx = midas::context("test", midas::dimension::DIM1);
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
    auto ctx = midas::context("test", midas::dimension::DIM3);
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

    auto ctx2 = midas::context("test", midas::dimension::NODIM);
    ASSERT_THROW(ctx2.reduce(), midas::dimension_error);
}

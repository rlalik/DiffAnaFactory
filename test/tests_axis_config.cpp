#include <gtest/gtest.h>

#include <midas.hpp>

TEST(TestsAxisConfig, CreationByValues)
{
    midas::axis_config ac1;
    ac1.set_bins(10, 0, 10);

    ASSERT_EQ(ac1.get_bins(), 10);
    ASSERT_EQ(ac1.get_min(), 0);
    ASSERT_EQ(ac1.get_max(), 10);

    Float_t observable;
    midas::axis_config ac2(&observable);
    ac2.set_bins(10, 0, 10);

    ASSERT_EQ(ac2.get_bins(), 10);
    ASSERT_EQ(ac2.get_min(), 0);
    ASSERT_EQ(ac2.get_max(), 10);
}

TEST(TestsAxisConfig, CreationByArray)
{
    midas::axis_config ac1;
    Double_t bins_x[] = {0.0, 1.1, 2.0, 3.0, 4.0, 10.0};
    ac1.set_bins(bins_x);

    ASSERT_EQ(ac1.get_bins(), 5);
}

#include <gtest/gtest.h>

#include "midas.hpp"

TEST(TestsAxisConfig, Creation)
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

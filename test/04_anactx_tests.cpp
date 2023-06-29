#include <cppunit/extensions/HelperMacros.h>

#include "midas.hpp"

class AnaCtxCase : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(AnaCtxCase);
    CPPUNIT_TEST(MyTest);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;

protected:
    void MyTest();

    int bins;
    float min, max;
    midas::axis_config axis;
    midas::v_context a3ctx;
};

CPPUNIT_TEST_SUITE_REGISTRATION(AnaCtxCase);

void AnaCtxCase::setUp()
{
    bins = 10;
    min = 0;
    max = 10;

    a3ctx.V.set_label("a2_V");
}

void AnaCtxCase::MyTest()
{
    std::string output_string;

    a3ctx.dim = midas::DIM2;
    a3ctx.name = "test";
    a3ctx.x.set_bins(10, 0, 10).set_label("a2_x").set_unit("mm");
    a3ctx.y.set_bins(10, 0, 10).set_label("a2_y");
    a3ctx.format_diff_axis();

    output_string = a3ctx.x.format_unit();
    CPPUNIT_ASSERT_EQUAL(std::string(" [mm]"), output_string);
    output_string = a3ctx.label;
    CPPUNIT_ASSERT_EQUAL(std::string("d^{2}/da2_xda2_y"), output_string);

    a3ctx.dim = midas::DIM3;
    a3ctx.z.set_bins(10, 0, 10).set_label("a2_z");
    a3ctx.format_diff_axis();

    output_string = a3ctx.label;
    CPPUNIT_ASSERT_EQUAL(std::string("d^{3}/da2_xda2_yda2_z"), output_string);

    midas::v_context a3ctx_2 = a3ctx;
    a3ctx_2.format_diff_axis();

    output_string = a3ctx_2.label;
    CPPUNIT_ASSERT_EQUAL(std::string("d^{3}/da2_xda2_yda2_z"), output_string);

    CPPUNIT_ASSERT_EQUAL(true, a3ctx == a3ctx_2);

    a3ctx_2.x.set_bins(13, 0, 10);
    CPPUNIT_ASSERT_EQUAL(false, a3ctx == a3ctx_2);

    CPPUNIT_ASSERT_EQUAL(true, a3ctx == a3ctx);
}

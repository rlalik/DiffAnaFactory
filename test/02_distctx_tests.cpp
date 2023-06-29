#include <cppunit/extensions/HelperMacros.h>

#include "midas.hpp"

class DistCtxCase : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(DistCtxCase);
    CPPUNIT_TEST(MyTest);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;

protected:
    void MyTest();

    int bins;
    float min, max;
    midas::axis_config axis;
    midas::context d3ctx;
};

CPPUNIT_TEST_SUITE_REGISTRATION(DistCtxCase);

void DistCtxCase::setUp()
{
    bins = 10;
    min = 0;
    max = 10;

    d3ctx.dim = midas::DIM3;
    d3ctx.name = "test";
    d3ctx.x.set_label("d2_x").set_unit("mm");
    d3ctx.y.set_label("d2_y").set_unit("");
    d3ctx.z.set_label("d2_z").set_unit("cm");

    //   d3ctx.print();
}

void DistCtxCase::MyTest()
{
    std::string output_string;

    output_string = d3ctx.z.format_unit();
    CPPUNIT_ASSERT_EQUAL(std::string(" [cm]"), output_string);

    midas::context d3ctx_2 = d3ctx;

    output_string = d3ctx_2.z.format_unit();
    CPPUNIT_ASSERT_EQUAL(std::string(" [cm]"), output_string);

    CPPUNIT_ASSERT_EQUAL(true, d3ctx == d3ctx_2);

    d3ctx_2.x.set_bins(13, 0, 0);
    CPPUNIT_ASSERT_EQUAL(false, d3ctx == d3ctx_2);

    CPPUNIT_ASSERT_EQUAL(true, d3ctx == d3ctx);
}

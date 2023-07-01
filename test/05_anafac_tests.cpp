#include <cppunit/extensions/HelperMacros.h>

#include "midas.hpp"

#include <TFile.h>

class AnaFacCase : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(AnaFacCase);
    CPPUNIT_TEST(InitializationTest);
    CPPUNIT_TEST(ScaleTest);
    CPPUNIT_TEST(FillTest);
    CPPUNIT_TEST(WriteTest);
    CPPUNIT_TEST(ReadTest);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

protected:
    void InitializationTest();
    void ScaleTest();
    void FillTest();
    void WriteTest();
    void ReadTest();

    int bins;
    float min, max;
    midas::v_context a2ctx;
    midas::v_context a3ctx;
    midas::DifferentialFactory* a2fac;
    midas::DifferentialFactory* a3fac;
};

CPPUNIT_TEST_SUITE_REGISTRATION(AnaFacCase);

void AnaFacCase::setUp()
{
    a3ctx = midas::v_context(midas::dimension::DIM3);
    a3ctx.x.set_bins(4, -10, 10).set_label("a3_x").set_unit("cm");
    a3ctx.y.set_bins(3, -10, 10).set_label("a3_y").set_unit("mm");
    a3ctx.z.set_bins(2, -10, 10).set_label("a3_z").set_unit("um");
    a3ctx.V.set_bins(100, -10, 10).set_label("a3_V");

    a2ctx = a3ctx;

    a2ctx = midas::v_context(midas::dimension::DIM2); // FIXME
    a2ctx.name = "a2fac";
    a2ctx.hist_name = "a2fac";

    a3ctx.name = "a3fac";
    a3ctx.hist_name = "a3fac";
}

void AnaFacCase::tearDown()
{
    if (a2fac) delete a2fac;
    a2fac = nullptr;
    if (a3fac) delete a3fac;
    a3fac = nullptr;
}

void AnaFacCase::InitializationTest()
{
    midas::ExtraDimensionMapper* edm;

    a2fac = new midas::DifferentialFactory(a2ctx);
    a2fac->init();

    a3fac = new midas::DifferentialFactory(a3ctx);
    a3fac->init();

    edm = a2fac->diffs;

    CPPUNIT_ASSERT_EQUAL(a2ctx.x.get_bins(), edm->getBinsX());
    CPPUNIT_ASSERT_EQUAL(a2ctx.y.get_bins(), edm->getBinsY());
    //   CPPUNIT_ASSERT_EQUAL(a2ctx.z.get_bins(), edm->getBinsZ());

    edm = a3fac->diffs;

    CPPUNIT_ASSERT_EQUAL(a3ctx.x.get_bins(), edm->getBinsX());
    CPPUNIT_ASSERT_EQUAL(a3ctx.y.get_bins(), edm->getBinsY());
    CPPUNIT_ASSERT_EQUAL(a3ctx.z.get_bins(), edm->getBinsZ());
}

void AnaFacCase::ScaleTest()
{
    a3fac = new midas::DifferentialFactory(a3ctx);
    a3fac->init();

    TH1* h = (*a3fac->diffs)[0];
    h->FillRandom("gaus");

    a3ctx.format_diff_axis();

    int n1 = h->GetBinContent(5);

    a3fac->scale(3);
    int n2 = h->GetBinContent(5);
    CPPUNIT_ASSERT_EQUAL(n1 * 3, n2);

    std::string output_string;
    output_string = a3ctx.label;
    CPPUNIT_ASSERT_EQUAL(std::string("d^{3}/da3_xda3_yda3_z"), output_string);
    a3fac->finalize();
}

void AnaFacCase::FillTest()
{
    a2fac = new midas::DifferentialFactory(a2ctx);
    a2fac->init();

    a3fac = new midas::DifferentialFactory(a3ctx);
    a3fac->init();

    midas::ExtraDimensionMapper* edm;

    edm = a2fac->diffs;

    CPPUNIT_ASSERT_EQUAL(a2ctx.x.get_bins(), edm->getBinsX());
    CPPUNIT_ASSERT_EQUAL(a2ctx.y.get_bins(), edm->getBinsY());
    //   CPPUNIT_ASSERT_EQUAL(a3ctx.z.get_bins(), edm->getBinsZ());

    edm = a3fac->diffs;

    CPPUNIT_ASSERT_EQUAL(a3ctx.x.get_bins(), edm->getBinsX());
    CPPUNIT_ASSERT_EQUAL(a3ctx.y.get_bins(), edm->getBinsY());
    CPPUNIT_ASSERT_EQUAL(a3ctx.z.get_bins(), edm->getBinsZ());
}

void AnaFacCase::WriteTest()
{
    a2fac = new midas::DifferentialFactory(a2ctx);
    a2fac->init();
    CPPUNIT_ASSERT_EQUAL(true, 0 != a2fac->ctx.validate());
    CPPUNIT_ASSERT_EQUAL(true, a2fac->ctx.update());
    (*a2fac->diffs)[0]->FillRandom("gaus", 1000);

    a3fac = new midas::DifferentialFactory(a3ctx);
    a3fac->init();
    CPPUNIT_ASSERT_EQUAL(true, 0 != a3fac->ctx.validate());
    CPPUNIT_ASSERT_EQUAL(true, a3fac->ctx.update());
    (*a3fac->diffs)[0]->FillRandom("gaus", 1000);

    TFile* file = TFile::Open("/tmp/res.root", "RECREATE");

    a2fac->ctx.Write();
    a2fac->write(file, true);

    a3fac->ctx.Write();
    a3fac->write(file, true);

    file->Close();
}

void AnaFacCase::ReadTest()
{
    TFile* file = TFile::Open("/tmp/res.root", "OPEN");
    if (!file or !file->IsOpen())
    {
        std::cerr << "File " << file << " can't be open!";
        return;
    }

    midas::v_context* dactx = nullptr;

    std::string tmpname = "a3fac";
    tmpname.append("Ctx");
    file->ls();
    dactx = (midas::v_context*)file->Get(tmpname.c_str());

    CPPUNIT_ASSERT_EQUAL(true, dactx != nullptr);
    dactx->print();

    midas::DifferentialFactory* fac = nullptr;
    fac = new midas::DifferentialFactory(dactx);
    fac->setSource(file);
    fac->init();

    //   fac->listRegisteredObjects();
    //   fac->diffs->listRegisteredObjects();

    CPPUNIT_ASSERT_EQUAL(1000, (int)(*fac->diffs)[0]->GetEntries());

    //   a3fac->MultiDimAnalysisExtension::ctx.Write();
    //   a3fac->write(file, true);
    file->Close();
}

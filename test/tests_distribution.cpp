#include <gtest/gtest.h>

#include <midas.hpp>

#include <TCanvas.h>
#include <TFile.h>
#include <TH1.h>

class TestsDistribution : public ::testing::Test
{
protected:
    void SetUp() override
    {
        box1 = std::unique_ptr<pandora::pandora>(new pandora::pandora("box1"));
        box2 = std::unique_ptr<pandora::pandora>(new pandora::pandora("box2"));
        box3 = std::unique_ptr<pandora::pandora>(new pandora::pandora("box3"));

        ctx_3d = std::unique_ptr<midas::context>(new midas::context("ctx_d3", midas::dimension::DIM3));
        ctx_3d->get_x().set_bins(4, -10, 10).set_label("a3_x").set_unit("cm").set_variable(&dummy_var);
        ctx_3d->get_y().set_bins(3, -10, 10).set_label("a3_y").set_unit("mm").set_variable(&dummy_var);
        ctx_3d->get_z().set_bins(2, -10, 10).set_label("a3_z").set_unit("um").set_variable(&dummy_var);
        ctx_3d->get_v().set_bins(1, -10, 10).set_label("a3_v").set_unit("").set_variable(&dummy_var);

        ctx_2d = std::unique_ptr<midas::context>(new midas::context(ctx_3d->cast("ctx_2d", midas::dimension::DIM2)));
        ctx_1d = std::unique_ptr<midas::context>(new midas::context(ctx_2d->cast("ctx_1d", midas::dimension::DIM1)));

        fac_1d = std::unique_ptr<midas::distribution>(new midas::distribution(*ctx_1d.get(), box1.get()));
        fac_2d = std::unique_ptr<midas::distribution>(new midas::distribution(*ctx_2d.get(), box2.get()));
        fac_3d = std::unique_ptr<midas::distribution>(new midas::distribution(*ctx_3d.get(), box3.get()));

        fac_1d->prepare();
        fac_2d->prepare();
        fac_3d->prepare();
    }

    // void TearDown() override {}

    std::unique_ptr<pandora::pandora> box1;
    std::unique_ptr<pandora::pandora> box2;
    std::unique_ptr<pandora::pandora> box3;

    std::unique_ptr<midas::context> ctx_1d;
    std::unique_ptr<midas::context> ctx_2d;
    std::unique_ptr<midas::context> ctx_3d;

    std::unique_ptr<midas::distribution> fac_1d;
    std::unique_ptr<midas::distribution> fac_2d;
    std::unique_ptr<midas::distribution> fac_3d;

    Float_t dummy_var;
};

/*
TEST_F(TestsDistribution, setUp)
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
*/
TEST_F(TestsDistribution, Operators)
{
    ASSERT_FALSE(*fac_1d == *fac_2d);
    ASSERT_FALSE(*fac_2d == *fac_3d);
    ASSERT_FALSE(*fac_3d == *fac_1d);

    ASSERT_TRUE(*fac_1d == *fac_1d);
    ASSERT_TRUE(*fac_2d == *fac_2d);
    ASSERT_TRUE(*fac_3d == *fac_3d);
}

// TEST_F(TestsDistribution, Scale)
// {
//     a3fac = new midas::distribution(a3ctx);
//     a3fac->init();
//
//     TH1* h = (*a3fac->diffs)[0];
//     h->FillRandom("gaus");
//
//     a3ctx.format_diff_axis();
//
//     int n1 = h->GetBinContent(5);
//
//     a3fac->scale(3);
//     int n2 = h->GetBinContent(5);
//     ASSERT_EQ(n1 * 3, n2);
//
//     std::string output_string;
//     output_string = a3ctx.label;
//     ASSERT_EQ(std::string("d^{3}/da3_xda3_yda3_z"), output_string);
//     a3fac->finalize();
// }
//
// TEST_F(TestsDistribution, Fill)
// {
//     a2fac = new midas::distribution(a2ctx);
//     a2fac->init();
//
//     a3fac = new midas::distribution(a3ctx);
//     a3fac->init();
//
//     midas::observable* edm;
//
//     edm = a2fac->diffs;
//
//     ASSERT_EQ(a2ctx.x.get_bins(), edm->getBinsX());
//     ASSERT_EQ(a2ctx.y.get_bins(), edm->getBinsY());
//     //   ASSERT_EQ(a3ctx.z.get_bins(), edm->getBinsZ());
//
//     edm = a3fac->diffs;
//
//     ASSERT_EQ(a3ctx.x.get_bins(), edm->getBinsX());
//     ASSERT_EQ(a3ctx.y.get_bins(), edm->getBinsY());
//     ASSERT_EQ(a3ctx.z.get_bins(), edm->getBinsZ());
// }
//
// TEST_F(TestsDistribution, Write)
// {
//     a2fac = new midas::distribution(a2ctx);
//     a2fac->init();
//     ASSERT_EQ(true, 0 != a2fac->ctx.validate());
//     ASSERT_EQ(true, a2fac->ctx.update());
//     (*a2fac->diffs)[0]->FillRandom("gaus", 1000);
//
//     a3fac = new midas::distribution(a3ctx);
//     a3fac->init();
//     ASSERT_EQ(true, 0 != a3fac->ctx.validate());
//     ASSERT_EQ(true, a3fac->ctx.update());
//     (*a3fac->diffs)[0]->FillRandom("gaus", 1000);
//
//     TFile* file = TFile::Open("/tmp/res.root", "RECREATE");
//
//     a2fac->ctx.Write();
//     a2fac->write(file, true);
//
//     a3fac->ctx.Write();
//     a3fac->write(file, true);
//
//     file->Close();
// }
//
// TEST_F(TestsDistribution, Read)
// {
//     TFile* file = TFile::Open("/tmp/res.root", "OPEN");
//     if (!file or !file->IsOpen())
//     {
//         std::cerr << "File " << file << " can't be open!";
//         return;
//     }
//
//     midas::context* dactx = nullptr;
//
//     std::string tmpname = "a3fac";
//     tmpname.append("Ctx");
//     file->ls();
//     dactx = (midas::context*)file->Get(tmpname.c_str());
//
//     ASSERT_EQ(true, dactx != nullptr);
//     dactx->print();
//
//     midas::distribution* fac = nullptr;
//     fac = new midas::distribution(dactx);
//     fac->setSource(file);
//     fac->init();
//
//     //   fac->listRegisteredObjects();
//     //   fac->diffs->listRegisteredObjects();
//
//     ASSERT_EQ(1000, (int)(*fac->diffs)[0]->GetEntries());
//
//     //   a3fac->MultiDimAnalysisExtension::ctx.Write();
//     //   a3fac->write(file, true);
//     file->Close();
// }

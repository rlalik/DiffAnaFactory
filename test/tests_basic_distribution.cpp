#include <gtest/gtest.h>

#include "midas.hpp"

#include <TCanvas.h>
#include <TFile.h>
#include <TH1.h>

#include <fmt/core.h>

class TestsBasicDistribution : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // q0_ remains empty
        ctx_3d = std::unique_ptr<midas::basic_context>(new midas::basic_context("ctx_d3", midas::dimension::DIM3));
        ctx_3d->get_x().set_bins(4, -10, 10).set_label("a3_x").set_unit("cm").set_variable(&dummy_var);
        ctx_3d->get_y().set_bins(3, -10, 10).set_label("a3_y").set_unit("mm").set_variable(&dummy_var);
        ctx_3d->get_z().set_bins(2, -10, 10).set_label("a3_z").set_unit("um").set_variable(&dummy_var);

        ctx_2d = std::unique_ptr<midas::basic_context>(
            new midas::basic_context(ctx_3d->cast("ctx_2d", midas::dimension::DIM2)));
        ctx_1d = std::unique_ptr<midas::basic_context>(
            new midas::basic_context(ctx_2d->cast("ctx_1d", midas::dimension::DIM1)));

        fac_1d = std::unique_ptr<midas::basic_distribution>(new midas::basic_distribution(ctx_1d.get()));
        fac_2d = std::unique_ptr<midas::basic_distribution>(new midas::basic_distribution(ctx_2d.get()));
        fac_3d = std::unique_ptr<midas::basic_distribution>(new midas::basic_distribution(ctx_3d.get()));

        fac_1d->prepare();
        fac_2d->prepare();
        fac_3d->prepare();
    }

    // void TearDown() override {}

    std::unique_ptr<midas::basic_context> ctx_1d;
    std::unique_ptr<midas::basic_context> ctx_2d;
    std::unique_ptr<midas::basic_context> ctx_3d;

    std::unique_ptr<midas::basic_distribution> fac_1d;
    std::unique_ptr<midas::basic_distribution> fac_2d;
    std::unique_ptr<midas::basic_distribution> fac_3d;

    Float_t dummy_var;
};

TEST_F(TestsBasicDistribution, Initialization)
{
    fac_3d->transform(
        [&](TH1* h)
        {
            ASSERT_EQ(h->GetNbinsX(), ctx_3d->get_x().get_bins());
            ASSERT_EQ(h->GetNbinsY(), ctx_3d->get_y().get_bins());
            ASSERT_EQ(h->GetNbinsZ(), ctx_3d->get_z().get_bins());
        });
}

TEST_F(TestsBasicDistribution, Scale)
{
    fac_1d->transform([](TH1* h) { h->FillRandom("gaus"); });

    Double_t n1;
    fac_1d->transform([&](TH1* h) { n1 = h->GetBinContent(5); });

    fac_1d->transform([](TH1* h) { h->Scale(3); });

    Double_t n2;
    fac_1d->transform([&](TH1* h) { n2 = h->GetBinContent(5); });
    ASSERT_EQ(n1 * 3, n2);

    fac_3d->finalize();
}

// TEST_F(TestsBasicDistribution, FillTest)
// {
//
//     ASSERT_EQ(ctx_2d.get_x().get_bins(), edm->getBinsX());
//     ASSERT_EQ(ctx_2d.get_y().get_bins(), edm->getBinsY());
//     //   ASSERT_EQ(ctx_3d.z.get_bins(), edm->getBinsZ());
//
//     edm = fac_3d->diffs;
//
//     ASSERT_EQ(ctx_3d.get_x().get_bins(), edm->getBinsX());
//     ASSERT_EQ(ctx_3d.get_y().get_bins(), edm->getBinsY());
//     ASSERT_EQ(ctx_3d.get_z().get_bins(), edm->getBinsZ());
// }
//
// TEST_F(TestsBasicDistribution, WriteTest)
// {
//     ASSERT_EQ(true, 0 != fac_2d.ctx.validate());
//     ASSERT_EQ(true, fac_2d.ctx.update());
//
//     ASSERT_EQ(true, 0 != fac_3d.ctx.validate());
//     ASSERT_EQ(true, fac_3d.ctx.update());
//
//     TFile* file = TFile::Open("/tmp/res.root", "RECREATE");
//
//     fac_2d.ctx.Write();
//     fac_2d.write(file, true);
//
//     fac_3d.ctx.Write();
//     fac_3d.write(file, true);
//
//     file->Close();
// }
//
// TEST_F(TestsBasicDistribution, ReadTest)
// {
//     TFile* file = TFile::Open("/tmp/res.root", "OPEN");
//     if (!file or !file->IsOpen())
//     {
//         std::cerr << "File " << file << " can't be open!";
//         return;
//     }
//
//     midas::v_basic_context* dactx = nullptr;
//
//     std::string tmpname = "fac_3d";
//     tmpname.append("Ctx");
//     file->ls();
//     dactx = (midas::v_basic_context*)file->Get(tmpname.c_str());
//
//     ASSERT_TRUE(dactx != nullptr);
//     dactx->print();
//
//     midas::DifferentialFactory* fac = nullptr;
//     fac = new midas::DifferentialFactory(dactx);
//     fac->setSource(file);
//     fac->init();
//
//     //   fac->listRegisteredObjects();
//     //   fac->diffs->listRegisteredObjects();
//
//     ASSERT_EQ(1000, (int)(*fac->diffs)[0]->GetEntries());
//
//     //   fac_3d->MultiDimAnalysisExtension::ctx.Write();
//     //   fac_3d->write(file, true);
//     file->Close();
// }

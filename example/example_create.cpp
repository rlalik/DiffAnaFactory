#include <midas.hpp>

#include <fmt/core.h>

#include <TCanvas.h>
#include <TFile.h>
#include <TH1.h>
#include <TStyle.h>

int main()
{
    auto f_lambda = TF1("f_lambda", "gaus(0)+pol5(3)", 1080, 1200);
    f_lambda.SetParameters(1000, 1116, 3, 1e-5, 1e-6, 1e-7, 1e-8, 1e-9);

    auto f_dist_x = TF1("f_dist_x", "pol0", -1, 1);
    f_dist_x.SetParameter(0, 1);

    auto f_dist_y = TF1("f_dist_y", "gaus(0)", 0, 1500);
    f_dist_y.SetParameters(1, 650, 300);

    Float_t p_cm;
    Float_t theta_cm;
    Float_t m_lambda;

    midas::basic_context ctx("pcm_costhcm", midas::dimension::DIM2);
    ctx.get_x().set_bins(100, -1, 1).set_label("cos_theta_cm").set_unit("rad").set_variable(&theta_cm);
    ctx.get_y().set_bins(100, 0, 1500).set_label("p_cm").set_unit("MeV/c").set_variable(&p_cm);
    ctx.prepare();
    // ctx.print();

    midas::context ctx_sig("pcm_costhcm_sig", midas::dimension::DIM2);
    ctx_sig.get_x().set_bins(10, -1, 1).set_label("cos_theta_cm").set_unit("rad").set_variable(&theta_cm);
    ctx_sig.get_y().set_bins(10, 0, 1500).set_label("p_cm").set_unit("MeV/c").set_variable(&p_cm);
    ctx_sig.get_v().set_bins(120, 1080, 1200).set_label("m_lambda").set_unit("MeV/c^2").set_variable(&m_lambda);
    ctx_sig.prepare();
    // ctx_sig.print();

    pandora::pandora box("test");
    pandora::pandora box_sig("test_sig");

    midas::basic_distribution dist(ctx, &box);
    dist.prepare();
    dist.print();

    midas::distribution dist_sig(ctx_sig, &box_sig);
    dist_sig.prepare();
    dist_sig.print();

    for (int i = 0; i < 10000000; ++i)
    {
        m_lambda = f_lambda.GetRandom();
        theta_cm = f_dist_x.GetRandom();
        p_cm = f_dist_y.GetRandom();
        dist.fill();
        dist_sig.fill();
    }

    dist.finalize();
    dist_sig.finalize();

    dist.transform([](TCanvas* c) { c->SetMargin(0.20f, 0.15f, 0.15f, 0.05f); });

    dist.transform(
        [](TH1* h)
        {
            h->GetXaxis()->SetLabelSize(0.06f);
            h->GetXaxis()->SetTitleSize(0.06f);
            h->GetXaxis()->SetNdivisions(505);

            h->GetYaxis()->SetLabelSize(0.06f);
            h->GetYaxis()->SetTitleSize(0.06f);

            h->Print();
        });

    dist.transform([](TCanvas* c) { c->Print(".png"); });

    box.list_registered_objects();
    box_sig.list_registered_objects();

    auto f = TFile::Open("example.root", "RECREATE");
    dist.save(f);
    dist_sig.save(f);
    f->Close();
}

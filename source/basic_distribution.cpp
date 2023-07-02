/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "midas.hpp"

#include "detail.hpp"

#include <fmt/core.h>

#include <TCanvas.h>
#include <TF1.h>
#include <TH2.h>
#include <TH3.h>
#include <TList.h>
#include <TSystem.h>

// const Option_t h1opts[] = "h,E1";

namespace midas
{

basic_distribution::basic_distribution()
    : RT::Pandora(""), ctx(basic_context("", dimension::NODIM)), hSignalCounter(nullptr), cSignalCounter(nullptr),
      drawOpts("colz")
{
    prepare();
    init();
}

basic_distribution::basic_distribution(const basic_context& context)
    : RT::Pandora(""), ctx(context), hSignalCounter(nullptr), cSignalCounter(nullptr), drawOpts("colz")
{
    prepare();
    init();
}

basic_distribution::basic_distribution(const basic_context* context)
    : RT::Pandora(""), ctx(*context), hSignalCounter(nullptr), cSignalCounter(nullptr), drawOpts("colz")
{
    prepare();
    init();
}

basic_distribution::~basic_distribution() { gSystem->ProcessEvents(); }
/*
distribution& distribution::operator=(const distribution& fa)
{
    if (this == &fa) return *this;

    static_cast<RT::Pandora>(*this) = static_cast<RT::Pandora>(fa);
    distribution* nthis = this; // new DistributionFactory(fa.ctx);

    nthis->ctx = fa.ctx;

    init();

    detail::copyHistogram(fa.hSignalCounter.get(), hSignalCounter.get());
    return *this;
}*/

void basic_distribution::prepare()
{
    ctx.update();
    rename(ctx.hist_name.Data());
    chdir(ctx.dir_name.Data());
}

void basic_distribution::init()
{
    TString htitle = ctx.format_hist_axes();
    TString htitlez = ctx.z.format_string();

    if (ctx.dim == dimension::NODIM) { throw dimension_error("Dimension not set"); }

    // input histograms
    if (dimension::DIM3 == ctx.dim && !hSignalCounter)
    {
        hSignalCounter = std::unique_ptr<TH1>(
            RegTH3<TH3D>("@@@d/h_@@@a", htitle, ctx.x.get_bins(), ctx.x.get_min(), ctx.x.get_max(), ctx.y.get_bins(),
                         ctx.y.get_min(), ctx.y.get_max(), ctx.z.get_bins(), ctx.z.get_min(), ctx.z.get_max()));
        hSignalCounter->SetTitle(ctx.context_title);
    }

    if (dimension::DIM2 == ctx.dim && !hSignalCounter)
    {
        hSignalCounter = std::unique_ptr<TH1>(
#ifdef HAVE_HISTASYMMERRORS
            RegTH2<TH2DA>("@@@d/h_@@@a", htitle,
#else
            RegTH2<TH2D>("@@@d/h_@@@a", htitle,
#endif
                          ctx.x.get_bins(), ctx.x.get_min(), ctx.x.get_max(), ctx.y.get_bins(), ctx.y.get_min(),
                          ctx.y.get_max()));
        hSignalCounter->SetTitle(ctx.context_title);
    }

    if (dimension::DIM1 == ctx.dim && !hSignalCounter)
    {
        hSignalCounter = std::unique_ptr<TH1>(
            RegTH1<TH1D>("@@@d/h_@@@a", htitle, ctx.x.get_bins(), ctx.x.get_min(), ctx.x.get_max()));
        hSignalCounter->SetTitle(ctx.context_title);
    }

    Int_t can_width = 800, can_height = 600;

    if (!cSignalCounter)
    {
        cSignalCounter = std::unique_ptr<TCanvas>(RegCanvas("@@@d/c_@@@a", htitle, can_width, can_height));

        cSignalCounter->SetTitle(ctx.context_title);
    }
}

void basic_distribution::reinit()
{
    if (!hSignalCounter) init();

    TString htitle_y = ctx.y.format_string();
    TString htitle_z = ctx.z.format_string();

    hSignalCounter->SetXTitle(ctx.x.format_string());
    hSignalCounter->SetYTitle(ctx.y.format_string());
    hSignalCounter->SetZTitle(ctx.z.format_string());
    hSignalCounter->SetTitle(ctx.context_title);

    cSignalCounter->SetTitle(ctx.context_title);

    rename(ctx.dir_name);
    rename(ctx.hist_name);
}

void basic_distribution::proceed()
{
    if (dimension::DIM3 == ctx.dim)
        dynamic_cast<TH3*>(hSignalCounter.get())
            ->Fill(*ctx.x.get_var(), *ctx.y.get_var(), *ctx.z.get_var(), *ctx.var_weight);

    if (dimension::DIM2 == ctx.dim)
        dynamic_cast<TH2*>(hSignalCounter.get())->Fill(*ctx.x.get_var(), *ctx.y.get_var(), *ctx.var_weight);

    if (dimension::DIM1 == ctx.dim) dynamic_cast<TH1*>(hSignalCounter.get())->Fill(*ctx.x.get_var(), *ctx.var_weight);

    // 	if (ctx.useClip() and
    // 			*ctx.x.get_var() > ctx.cx.min and *ctx.x.get_var() < ctx.cx.max and
    // 			*ctx.y.get_var() > ctx.cy.min and *ctx.y.get_var() < ctx.cy.max)
    // 	{
    // 		const Int_t xcbin = Int_t( (*ctx.x.get_var() - ctx.cx.min)/ctx.cx.delta );
    // 		const Int_t ycbin = Int_t( (*ctx.y.get_var() - ctx.cy.min)/ctx.cy.delta );
    //
    // 		hDiscreteXYDiff[xcbin][ycbin]->Fill(*ctx.V.get_var(), *ctx.var_weight);
    //
    // 		isInRange = kTRUE;
    // 	}

    // 	if (ctx.useCuts() and *ctx.V.get_var() > ctx.cutMin and *ctx.V.get_var() < ctx.cutMax)
    // 	{
    // 		hSignalWithCutsXY->Fill(*ctx.x.get_var(), *ctx.y.get_var(), *ctx.var_weight);
    //
    // 		if (isInRange)
    // 		{
    // 			hDiscreteXY->Fill(*ctx.x.get_var(), *ctx.y.get_var(), *ctx.var_weight);
    // 		}
    // 	}
}

auto basic_distribution::transform(std::function<void(TH1* h)> transform_function) -> void
{
    transform_function(hSignalCounter.get());
}

auto basic_distribution::transform(std::function<void(TCanvas* h)> transform_function) -> void
{
    transform_function(cSignalCounter.get());
}

void basic_distribution::finalize(const char* draw_opts) { prepareCanvas(draw_opts); }

void basic_distribution::prepareCanvas(const char* draw_opts)
{
    TString colzopts = draw_opts ? TString(draw_opts) : drawOpts;
    TString coltopts = "col,text";

    hSignalCounter->GetXaxis()->SetTitle(ctx.x.format_string());
    if (dimension::DIM1 == ctx.dim) { hSignalCounter->GetYaxis()->SetTitle(ctx.context_axis_text.Data()); }
    else if (dimension::DIM2 == ctx.dim)
    {
        hSignalCounter->GetYaxis()->SetTitle(ctx.y.format_string());
        hSignalCounter->GetZaxis()->SetTitle(ctx.context_axis_text.Data());
    }
    else if (dimension::DIM3 == ctx.dim)
    {
        hSignalCounter->GetYaxis()->SetTitle(ctx.y.format_string());
        hSignalCounter->GetZaxis()->SetTitle(ctx.z.format_string());
        hSignalCounter->SetTitle(ctx.context_axis_text);
    }

    cSignalCounter->cd();
    hSignalCounter->Draw(colzopts);
    hSignalCounter->SetMarkerColor(kWhite);
    hSignalCounter->SetMarkerSize(1.6f);
    hSignalCounter->Sumw2();

    // RT::NicePalette(dynamic_cast<TH2*>(hSignalCounter.get()), 0.05f);
    // RT::NoPalette(dynamic_cast<TH2*>(hSignalCounter.get()));
    gPad->Update();
}

// TODO this two should be moved somewhere else, not in library
void basic_distribution::applyAngDists(double a2, double a4, double corr_a2, double corr_a4)
{
    const size_t hists_num = 1;
    TH1* hist_to_map[hists_num] = {hSignalCounter.get()};

    for (size_t x = 0; x < hists_num; ++x)
        applyAngDists(hist_to_map[x], a2, a4, corr_a2, corr_a4);
}

void basic_distribution::applyAngDists(TH1* h, double a2, double a4, double corr_a2, double corr_a4)
{
    auto f = new TF1("local_legpol", "angdist", -1, 1);
    f->SetParameter(0, 1.0);
    f->SetParameter(1, a2);
    f->SetParameter(2, a4);

    bool has_corr = false;
    TF1* f_corr = nullptr;
    if (corr_a2 != 0.0 or corr_a4 != 0.0)
    {
        has_corr = true;
        f_corr = new TF1("local_legpol_corr", "angdist", -1, 1);

        f_corr->SetParameter(0, 1.0);
        f_corr->SetParameter(1, corr_a2);
        f_corr->SetParameter(2, corr_a4);
    }

    auto bins_x = h->GetXaxis()->GetNbins();
    auto bins_y = h->GetYaxis()->GetNbins();

    for (auto x = 1; x <= bins_x; ++x)
    {
        auto bin_l = h->GetXaxis()->GetBinLowEdge(x);
        auto bin_r = h->GetXaxis()->GetBinUpEdge(x);
        auto bin_w = bin_r - bin_l;
        auto corr_factor = 1.0;

        auto angmap = f->Integral(bin_l, bin_r);

        if (has_corr) { corr_factor = f_corr->Integral(bin_l, bin_r); }
        else { angmap /= bin_w; }

        auto scaling_factor = angmap / corr_factor;
        for (auto y = 1; y <= bins_y; ++y)
        {
            auto tmp_val = h->GetBinContent(x, y);
            h->SetBinContent(x, y, tmp_val * scaling_factor);
        }
    }
    f->Delete();
    if (f_corr) f_corr->Delete();
}

// TODO move away
void basic_distribution::applyBinomErrors(TH1* N)
{
    constexpr size_t hists_num = 1;
    const std::array<TH1*, hists_num> hmap = {hSignalCounter.get()};

    for (size_t x = 0; x < hists_num; ++x)
        applyBinomErrors(hmap[x], N);
}

// TODO move away
void basic_distribution::applyBinomErrors(TH1* q, TH1* N)
{ /*RT::calcBinomialErrors(q, N);*/
}

void basic_distribution::rename(const char* newname) { Pandora::rename(newname); }

void basic_distribution::chdir(const char* newdir) { Pandora::chdir(newdir); }

auto basic_distribution::print() const -> void
{
    fmt::print("Distribution info:\n");
    ctx.print();
    hSignalCounter->Print();
}

}; // namespace midas

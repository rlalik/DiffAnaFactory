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

#include <RootTools.h>

#include <fmt/core.h>

#include <TCanvas.h>
#include <TF1.h>
#include <TH3.h>
#include <TList.h>
#include <TSystem.h>

// const Option_t h1opts[] = "h,E1";

namespace midas
{

distribution::distribution()
    : RT::Pandora(""), ctx(context("", dimension::NODIM)), hSignalCounter(nullptr), cSignalCounter(nullptr),
      drawOpts("colz")
{
    prepare();
    init();
}

distribution::distribution(const context& context)
    : RT::Pandora(""), ctx(context), hSignalCounter(nullptr), cSignalCounter(nullptr), drawOpts("colz")
{
    prepare();
    init();
}

distribution::distribution(const context* context)
    : RT::Pandora(""), ctx(*context), hSignalCounter(nullptr), cSignalCounter(nullptr), drawOpts("colz")
{
    prepare();
    init();
}

distribution::~distribution() { gSystem->ProcessEvents(); }
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

void distribution::prepare()
{
    ctx.update();
    rename(ctx.hist_name.Data());
    chdir(ctx.dir_name.Data());
}

void distribution::init()
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

void distribution::reinit()
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

void distribution::proceed()
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

auto distribution::transform(std::function<void(TH1* h)> transform_function) -> void {
    transform_function(hSignalCounter.get());
}

auto distribution::transform(std::function<void(TCanvas* h)> transform_function) -> void {
    transform_function(cSignalCounter.get());
}

void distribution::binnorm()
{
    if (hSignalCounter)
        hSignalCounter->Scale(
            1.0 / (hSignalCounter->GetXaxis()->GetBinWidth(1) * hSignalCounter->GetYaxis()->GetBinWidth(1)));
    //
    // 	// Signal with cut
    // 	if (ctx.useCuts())
    // 	{
    // 		if (hSignalWithCutsXY) hSignalWithCutsXY->Scale( 1.0 / (
    // hSignalWithCutsXY->GetXaxis()->GetBinWidth(1) * hSignalWithCutsXY->GetYaxis()->GetBinWidth(1)
    // ) );
    // 	}
    //
    // 	if (ctx.useClip())
    // 	{
    // 		if (hDiscreteXY) hDiscreteXY->Scale( 1.0 / ( hDiscreteXY->GetXaxis()->GetBinWidth(1) *
    // hDiscreteXY->GetYaxis()->GetBinWidth(1) ) ); 		if (hSignalCounter) hSignalCounter->Scale( 1.0
    // / ( hSignalCounter->GetXaxis()->GetBinWidth(1) * hSignalCounter->GetYaxis()->GetBinWidth(1) )
    // );
    //
    // // 		if (ctx.useDiff())
    // // 		{
    // // 			for (uint i = 0; i < ctx.cx.bins; ++i)
    // // 			{
    // // 				for (uint j = 0; j < ctx.cy.bins; ++j)
    // // 				{
    // // 					if (hDiscreteXYDiff) hDiscreteXYDiff[i][j]->Scale(factor);
    // // 				}
    // // 				if (hSliceXYDiff) hSliceXYDiff[i]->Scale(factor);
    // // 			}
    // // 		}
    // 	}
}
/*
void distribution::scale(Float_t factor)
{
    if (hSignalCounter) hSignalCounter->Scale(factor);
}*/

void distribution::finalize(const char* draw_opts) { prepareCanvas(draw_opts); }

void distribution::niceHisto(TVirtualPad* pad, TH1* hist, float mt, float mr, float mb, float ml, int ndivx, int ndivy,
                             float xls, float xts, float xto, float yls, float yts, float yto, bool centerY,
                             bool centerX)
{
    RT::Hist::NicePad(pad, mt, mr, mb, ml);
    RT::Hist::NiceHistogram(hist, ndivx, ndivy, xls, 0.005f, xts, xto, yls, 0.005f, yts, yto, centerY, centerX);
}

void distribution::niceHists(RT::Hist::PadFormat pf, const RT::Hist::GraphFormat& format)
{
    RT::Hist::NicePad(cSignalCounter->cd(), pf);
    RT::Hist::NiceHistogram(dynamic_cast<TH2*>(hSignalCounter.get()), format);
    hSignalCounter->GetYaxis()->CenterTitle(kTRUE);
}

void distribution::prepareCanvas(const char* draw_opts)
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
void distribution::applyAngDists(double a2, double a4, double corr_a2, double corr_a4)
{
    const size_t hists_num = 1;
    TH1* hist_to_map[hists_num] = {hSignalCounter.get()};

    for (size_t x = 0; x < hists_num; ++x)
        applyAngDists(hist_to_map[x], a2, a4, corr_a2, corr_a4);
}

void distribution::applyAngDists(TH1* h, double a2, double a4, double corr_a2, double corr_a4)
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
void distribution::applyBinomErrors(TH1* N)
{
    constexpr size_t hists_num = 1;
    const std::array<TH1*, hists_num> hmap = {hSignalCounter.get()};

    for (size_t x = 0; x < hists_num; ++x)
        applyBinomErrors(hmap[x], N);
}

// TODO move away
void distribution::applyBinomErrors(TH1* q, TH1* N) { RT::calcBinomialErrors(q, N); }

void distribution::rename(const char* newname) { Pandora::rename(newname); }

void distribution::chdir(const char* newdir) { Pandora::chdir(newdir); }

auto distribution::print() const -> void {
    fmt::print("Distribution info:\n");
    ctx.print();
    hSignalCounter->Print();
}

}; // namespace midas

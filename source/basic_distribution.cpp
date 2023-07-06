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
#include <TFile.h>
#include <TH2.h>
#include <TH3.h>
#include <TList.h>
#include <TSystem.h>

// const Option_t h1opts[] = "h,E1";

namespace midas
{

// basic_distribution::basic_distribution()
//     : pandora::pandora(""), ctx(basic_context("", dimension::NODIM)), signal_histogram(nullptr),
//     signal_canvas(nullptr),
//       drawOpts("colz")
// {
// }

basic_distribution::basic_distribution(const basic_context& context, pandora::pandora* pbox)
    : box(pbox), ctx(context), signal_histogram(nullptr), signal_canvas(nullptr), drawOpts("colz")
{
    if (box)
    {
        box->add_placeholder("{dir}", ctx.name.Data());
        box->add_placeholder("{analysis}", ctx.name.Data());
    }
}

basic_distribution::~basic_distribution()
{
    gSystem->ProcessEvents();
    for (auto& o : garbage)
        delete o;
}

void basic_distribution::prepare()
{
    // RegObject(&ctx);
    ctx.prepare();

    TString htitle = ctx.format_hist_axes();
    TString htitlez = ctx.z.format_string();

    if (ctx.dim == dimension::NODIM) { throw dimension_error("Dimension not set"); }

    Int_t can_width = 800, can_height = 600;
    signal_canvas = std::unique_ptr<TCanvas>(reg_canvas("{dir}/c_{analysis}", htitle, can_width, can_height));
    signal_canvas->SetTitle(ctx.context_title);

    int bins_mask = 0x00;
    if (ctx.x.get_bins_array())
        bins_mask |= (2 << 0);
    else if (ctx.x.get_bins())
        bins_mask |= (1 << 0);

    if (ctx.dim == dimension::DIM1)
    {
        switch (bins_mask)
        {
            case 0x01:
                signal_histogram = std::unique_ptr<TH1>(reg_hist<TH1D>(
                    "{dir}/h_{analysis}", htitle.Data(), ctx.x.get_bins(), ctx.x.get_min(), ctx.x.get_max()));
                break;
            case 0x02:
                signal_histogram = std::unique_ptr<TH1>(
                    reg_hist<TH1D>("{dir}/h_{analysis}", htitle.Data(), ctx.x.get_bins(), ctx.x.get_bins_array()));
                break;
        }
        signal_histogram->SetTitle(ctx.context_title);
        return;
    }

#ifdef HAVE_HISTASYMMERRORS
#define TH2D_TYPE TH2DA
#else
#define TH2D_TYPE TH2D
#endif
    if (ctx.y.get_bins_array())
        bins_mask |= (2 << 4);
    else if (ctx.y.get_bins())
        bins_mask |= (1 << 4);

    if (ctx.dim == dimension::DIM2)
    {
        switch (bins_mask)
        {
            case 0x11:
                signal_histogram = std::unique_ptr<TH1>(
                    reg_hist<TH2D_TYPE>("{dir}/h_{analysis}", htitle.Data(), ctx.x.get_bins(), ctx.x.get_min(),
                                        ctx.x.get_max(), ctx.y.get_bins(), ctx.y.get_min(), ctx.y.get_max()));
                break;
            case 0x12:
                signal_histogram = std::unique_ptr<TH1>(
                    reg_hist<TH2D_TYPE>("{dir}/h_{analysis}", htitle.Data(), ctx.x.get_bins(), ctx.x.get_bins_array(),
                                        ctx.y.get_bins(), ctx.y.get_min(), ctx.y.get_max()));
                break;
            case 0x21:
                signal_histogram = std::unique_ptr<TH1>(
                    reg_hist<TH2D_TYPE>("{dir}/h_{analysis}", htitle.Data(), ctx.x.get_bins(), ctx.x.get_min(),
                                        ctx.x.get_max(), ctx.y.get_bins(), ctx.y.get_bins_array()));
                break;
            case 0x22:
                signal_histogram = std::unique_ptr<TH1>(reg_hist<TH2D_TYPE>("{dir}/h_{analysis}", htitle.Data(),
                                                                            ctx.x.get_bins(), ctx.x.get_bins_array(),
                                                                            ctx.y.get_bins(), ctx.y.get_bins_array()));
                break;
        }
        signal_histogram->SetTitle(ctx.context_title);
        return;
    }

    if (ctx.z.get_bins_array())
        bins_mask |= (2 << 8);
    else if (ctx.z.get_bins())
        bins_mask |= (1 << 8);

    if (ctx.dim == dimension::DIM3)
    {
        switch (bins_mask)
        {
            case 0x111:
                signal_histogram = std::unique_ptr<TH1>(
                    reg_hist<TH3D>("{dir}/h_{analysis}", htitle.Data(), ctx.x.get_bins(), ctx.x.get_min(),
                                   ctx.x.get_max(), ctx.y.get_bins(), ctx.y.get_min(), ctx.y.get_max(),
                                   ctx.z.get_bins(), ctx.z.get_min(), ctx.z.get_max()));
                break;
            case 0x222:
                signal_histogram = std::unique_ptr<TH1>(
                    reg_hist<TH3D>("{dir}/h_{analysis}", htitle.Data(), ctx.x.get_bins(), ctx.x.get_bins_array(),
                                   ctx.y.get_bins(), ctx.y.get_bins_array(), ctx.z.get_bins(), ctx.z.get_bins_array()));
                break;
            default:
                throw std::runtime_error("TH3 must be all bins or all arrays. Mixed types provided.");
                break;
        }
        signal_histogram->SetTitle(ctx.context_title);
        return;
    }
}

void basic_distribution::reinit()
{
    TString htitle_y = ctx.y.format_string();
    TString htitle_z = ctx.z.format_string();

    signal_histogram->SetXTitle(ctx.x.format_string());
    signal_histogram->SetYTitle(ctx.y.format_string());
    signal_histogram->SetZTitle(ctx.z.format_string());
    signal_histogram->SetTitle(ctx.context_title);

    signal_canvas->SetTitle(ctx.context_title);
}

void basic_distribution::fill(Float_t weight)
{
    if (dimension::DIM3 == ctx.dim)
        dynamic_cast<TH3*>(signal_histogram.get())->Fill(*ctx.x.get_var(), *ctx.y.get_var(), *ctx.z.get_var(), weight);

    if (dimension::DIM2 == ctx.dim)
        dynamic_cast<TH2*>(signal_histogram.get())->Fill(*ctx.x.get_var(), *ctx.y.get_var(), weight);

    if (dimension::DIM1 == ctx.dim) dynamic_cast<TH1*>(signal_histogram.get())->Fill(*ctx.x.get_var(), weight);

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

void basic_distribution::finalize(const char* draw_opts) { prepare_canvas(draw_opts); }

auto basic_distribution::transform(std::function<void(TH1* h)> transform_function) -> void
{
    transform_function(signal_histogram.get());
}

auto basic_distribution::transform(std::function<void(TCanvas* h)> transform_function) -> void
{
    transform_function(signal_canvas.get());
}

void basic_distribution::prepare_canvas(const char* draw_opts)
{
    TString colzopts = draw_opts ? TString(draw_opts) : drawOpts;
    TString coltopts = "col,text";

    signal_histogram->GetXaxis()->SetTitle(ctx.x.format_string());
    if (dimension::DIM1 == ctx.dim) { signal_histogram->GetYaxis()->SetTitle(ctx.context_axis_text.Data()); }
    else if (dimension::DIM2 == ctx.dim)
    {
        signal_histogram->GetYaxis()->SetTitle(ctx.y.format_string());
        signal_histogram->GetZaxis()->SetTitle(ctx.context_axis_text.Data());
    }
    else if (dimension::DIM3 == ctx.dim)
    {
        signal_histogram->GetYaxis()->SetTitle(ctx.y.format_string());
        signal_histogram->GetZaxis()->SetTitle(ctx.z.format_string());
        signal_histogram->SetTitle(ctx.context_axis_text);
    }

    signal_canvas->cd();
    signal_histogram->Draw(colzopts);
    signal_histogram->SetMarkerColor(kWhite);
    signal_histogram->SetMarkerSize(1.6f);
    signal_histogram->Sumw2();

    // RT::NicePalette(dynamic_cast<TH2*>(signal_histogram.get()), 0.05f);
    // RT::NoPalette(dynamic_cast<TH2*>(signal_histogram.get()));
    gPad->Update();
}

// TODO move away
void basic_distribution::applyBinomErrors(TH1* N)
{
    constexpr size_t hists_num = 1;
    const std::array<TH1*, hists_num> hmap = {signal_histogram.get()};

    for (size_t x = 0; x < hists_num; ++x)
        applyBinomErrors(hmap[x], N);
}

// TODO move away
void basic_distribution::applyBinomErrors(TH1* q, TH1* N)
{ /*RT::calcBinomialErrors(q, N);*/
}

auto basic_distribution::print() const -> void
{
    fmt::print("Distribution info:\n");
    ctx.print();
    signal_histogram->Print();
}

auto basic_distribution::validate() const -> bool
{
    if (ctx.dim >= dimension::DIM1)
    {
        if (!ctx.x.get_var()) throw std::runtime_error("The x-axis variable is not set");
    }
    if (ctx.dim >= dimension::DIM2)
    {
        if (!ctx.y.get_var()) throw std::runtime_error("The x-axis variable is not set");
    }
    if (ctx.dim >= dimension::DIM3)
    {
        if (!ctx.z.get_var()) throw std::runtime_error("The x-axis variable is not set");
    }
    return true;
}

auto basic_distribution::save(const char* filename, bool verbose) -> bool
{
    auto f = TFile::Open(filename, "RECREATE");
    auto res = save(f, verbose);
    f->Close();
    return res;
}

auto basic_distribution::save(TFile* f, bool verbose) -> bool
{
    if (box) return box->export_structure(f, verbose);
    return false;
}

}; // namespace midas

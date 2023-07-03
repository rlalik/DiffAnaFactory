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
}

basic_distribution::basic_distribution(const basic_context& context)
    : RT::Pandora(""), ctx(context), hSignalCounter(nullptr), cSignalCounter(nullptr), drawOpts("colz")
{
}

basic_distribution::basic_distribution(const basic_context* context)
    : RT::Pandora(""), ctx(*context), hSignalCounter(nullptr), cSignalCounter(nullptr), drawOpts("colz")
{
}

basic_distribution::~basic_distribution() { gSystem->ProcessEvents(); }

void basic_distribution::prepare()
{
    // RegObject(&ctx);
    ctx.prepare();
    rename(ctx.hist_name.Data());
    chdir(ctx.dir_name.Data());

    TString htitle = ctx.format_hist_axes();
    TString htitlez = ctx.z.format_string();

    if (ctx.dim == dimension::NODIM) { throw dimension_error("Dimension not set"); }

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
                hSignalCounter = std::unique_ptr<TH1>(
                    RegHist<TH1D>("@@@d/h_@@@a", htitle.Data(), ctx.x.get_bins(), ctx.x.get_min(), ctx.x.get_max()));
                break;
            case 0x02:
                hSignalCounter = std::unique_ptr<TH1>(
                    RegHist<TH1D>("@@@d/h_@@@a", htitle.Data(), ctx.x.get_bins(), ctx.x.get_bins_array()));
                break;
        }
        hSignalCounter->SetTitle(ctx.context_title);
    }
    else
    {
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
                    hSignalCounter = std::unique_ptr<TH1>(
                        RegHist<TH2D_TYPE>("@@@d/h_@@@a", htitle.Data(), ctx.x.get_bins(), ctx.x.get_min(),
                                           ctx.x.get_max(), ctx.y.get_bins(), ctx.y.get_min(), ctx.y.get_max()));
                    break;
                case 0x12:
                    hSignalCounter = std::unique_ptr<TH1>(
                        RegHist<TH2D_TYPE>("@@@d/h_@@@a", htitle.Data(), ctx.x.get_bins(), ctx.x.get_bins_array(),
                                           ctx.y.get_bins(), ctx.y.get_min(), ctx.y.get_max()));
                    break;
                case 0x21:
                    hSignalCounter = std::unique_ptr<TH1>(
                        RegHist<TH2D_TYPE>("@@@d/h_@@@a", htitle.Data(), ctx.x.get_bins(), ctx.x.get_min(),
                                           ctx.x.get_max(), ctx.y.get_bins(), ctx.y.get_bins_array()));
                    break;
                case 0x22:
                    hSignalCounter = std::unique_ptr<TH1>(RegHist<TH2D_TYPE>("@@@d/h_@@@a", htitle.Data(),
                                                                             ctx.x.get_bins(), ctx.x.get_bins_array(),
                                                                             ctx.y.get_bins(), ctx.y.get_bins_array()));
                    break;
            }
            hSignalCounter->SetTitle(ctx.context_title);
        }
        else
        {
            if (ctx.z.get_bins_array())
                bins_mask |= (2 << 8);
            else if (ctx.z.get_bins())
                bins_mask |= (1 << 8);

            if (ctx.dim == dimension::DIM3)
            {
                switch (bins_mask)
                {
                    case 0x111:
                        hSignalCounter = std::unique_ptr<TH1>(
                            RegHist<TH3D>("@@@d/h_@@@a", htitle.Data(), ctx.x.get_bins(), ctx.x.get_min(),
                                          ctx.x.get_max(), ctx.y.get_bins(), ctx.y.get_min(), ctx.y.get_max(),
                                          ctx.z.get_bins(), ctx.z.get_min(), ctx.z.get_max()));
                        break;
                    case 0x222:
                        hSignalCounter = std::unique_ptr<TH1>(RegHist<TH3D>(
                            "@@@d/h_@@@a", htitle.Data(), ctx.x.get_bins(), ctx.x.get_bins_array(), ctx.y.get_bins(),
                            ctx.y.get_bins_array(), ctx.z.get_bins(), ctx.z.get_bins_array()));
                        break;
                    default:
                        throw std::runtime_error("TH3 must be all bins or all arrays. Mixed types provided.");
                        break;
                }
                hSignalCounter->SetTitle(ctx.context_title);
            }
        }
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

void basic_distribution::fill(Float_t weight)
{
    if (dimension::DIM3 == ctx.dim)
        dynamic_cast<TH3*>(hSignalCounter.get())->Fill(*ctx.x.get_var(), *ctx.y.get_var(), *ctx.z.get_var(), weight);

    if (dimension::DIM2 == ctx.dim)
        dynamic_cast<TH2*>(hSignalCounter.get())->Fill(*ctx.x.get_var(), *ctx.y.get_var(), weight);

    if (dimension::DIM1 == ctx.dim) dynamic_cast<TH1*>(hSignalCounter.get())->Fill(*ctx.x.get_var(), weight);

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
    transform_function(hSignalCounter.get());
}

auto basic_distribution::transform(std::function<void(TCanvas* h)> transform_function) -> void
{
    transform_function(cSignalCounter.get());
}

void basic_distribution::prepare_canvas(const char* draw_opts)
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

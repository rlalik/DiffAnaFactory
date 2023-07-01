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

#include <RootTools.h>

#include <TCanvas.h>
#include <TF1.h>
#include <TH3.h>
#include <TList.h>
#include <TSystem.h>

#define PR(x) std::cout << "++DEBUG: " << #x << " = |" << x << "| (" << __FILE__ << ", " << __LINE__ << ")\n";

// const Option_t h1opts[] = "h,E1";

namespace midas
{

DistributionFactory::DistributionFactory()
    : RT::Pandora(""), ctx(context("", dimension::NODIM)), hSignalCounter(nullptr), cSignalCounter(nullptr),
      drawOpts("colz")
{
    prepare();
}

DistributionFactory::DistributionFactory(const context& context)
    : RT::Pandora(""), ctx(context), hSignalCounter(nullptr), cSignalCounter(nullptr), drawOpts("colz")
{
    prepare();
}

DistributionFactory::DistributionFactory(const context* context)
    : RT::Pandora(""), ctx(*context), hSignalCounter(nullptr), cSignalCounter(nullptr), drawOpts("colz")
{
    prepare();
}

DistributionFactory::~DistributionFactory() { gSystem->ProcessEvents(); }

DistributionFactory& DistributionFactory::operator=(const DistributionFactory& fa)
{
    if (this == &fa) return *this;

    (RT::Pandora)(*this) = (RT::Pandora)fa;
    DistributionFactory* nthis = this; // new DistributionFactory(fa.ctx);

    nthis->ctx = fa.ctx;

    init();

    copyHistogram(fa.hSignalCounter, hSignalCounter);
    return *this;
}

void DistributionFactory::prepare()
{
    ctx.update();
    rename(ctx.hist_name.Data());
    chdir(ctx.dir_name.Data());
}

void DistributionFactory::init()
{
    TString htitle = ctx.format_hist_axes();
    TString htitlez = ctx.z.format_string();

    if (ctx.dim == dimension::NODIM) { throw dimension_error("Dimension not set"); }

    // input histograms
    if (dimension::DIM3 == ctx.dim && !hSignalCounter)
    {
        hSignalCounter =
            RegTH3<TH3D>("@@@d/h_@@@a", htitle, ctx.x.get_bins(), ctx.x.get_min(), ctx.x.get_max(), ctx.y.get_bins(),
                         ctx.y.get_min(), ctx.y.get_max(), ctx.z.get_bins(), ctx.z.get_min(), ctx.z.get_max());
        hSignalCounter->SetTitle(ctx.title);
    }

    if (dimension::DIM2 == ctx.dim && !hSignalCounter)
    {
#ifdef HAVE_HISTASYMMERRORS
        hSignalCounter = RegTH2<TH2DA>("@@@d/h_@@@a", htitle,
#else
        hSignalCounter = RegTH2<TH2D>("@@@d/h_@@@a", htitle,
#endif
                                       ctx.x.get_bins(), ctx.x.get_min(), ctx.x.get_max(), ctx.y.get_bins(),
                                       ctx.y.get_min(), ctx.y.get_max());
        hSignalCounter->SetTitle(ctx.title);
    }

    if (dimension::DIM1 == ctx.dim && !hSignalCounter)
    {
        hSignalCounter = RegTH1<TH1D>("@@@d/h_@@@a", htitle, ctx.x.get_bins(), ctx.x.get_min(), ctx.x.get_max());
        hSignalCounter->SetTitle(ctx.title);
    }

    Int_t can_width = 800, can_height = 600;

    if (!cSignalCounter)
    {
        cSignalCounter = RegCanvas("@@@d/c_@@@a", htitle, can_width, can_height);

        cSignalCounter->SetTitle(ctx.title);
    }
}

void DistributionFactory::reinit()
{
    if (!hSignalCounter) init();

    TString htitle_y = ctx.y.format_string();
    TString htitle_z = ctx.z.format_string();

    hSignalCounter->SetXTitle(ctx.x.format_string());
    hSignalCounter->SetYTitle(ctx.y.format_string());
    hSignalCounter->SetZTitle(ctx.z.format_string());
    hSignalCounter->SetTitle(ctx.title);

    cSignalCounter->SetTitle(ctx.title);

    rename(ctx.dir_name);
    rename(ctx.hist_name);
}

void DistributionFactory::proceed()
{
    if (dimension::DIM3 == ctx.dim)
        ((TH3*)hSignalCounter)->Fill(*ctx.x.get_var(), *ctx.y.get_var(), *ctx.z.get_var(), *ctx.var_weight);

    if (dimension::DIM2 == ctx.dim) ((TH2*)hSignalCounter)->Fill(*ctx.x.get_var(), *ctx.y.get_var(), *ctx.var_weight);

    if (dimension::DIM1 == ctx.dim) ((TH1*)hSignalCounter)->Fill(*ctx.x.get_var(), *ctx.var_weight);

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

void DistributionFactory::binnorm()
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

void DistributionFactory::scale(Float_t factor)
{
    if (hSignalCounter) hSignalCounter->Scale(factor);
}

void DistributionFactory::finalize(const char* draw_opts) { prepareCanvas(draw_opts); }

void DistributionFactory::niceHisto(TVirtualPad* pad, TH1* hist, float mt, float mr, float mb, float ml, int ndivx,
                                    int ndivy, float xls, float xts, float xto, float yls, float yts, float yto,
                                    bool centerY, bool centerX)
{
    RT::Hist::NicePad(pad, mt, mr, mb, ml);
    RT::Hist::NiceHistogram(hist, ndivx, ndivy, xls, 0.005, xts, xto, yls, 0.005, yts, yto, centerY, centerX);
}

void DistributionFactory::niceHists(RT::Hist::PadFormat pf, const RT::Hist::GraphFormat& format)
{
    RT::Hist::NicePad(cSignalCounter->cd(), pf);
    RT::Hist::NiceHistogram((TH2*)hSignalCounter, format);
    hSignalCounter->GetYaxis()->CenterTitle(kTRUE);
}

void DistributionFactory::prepareCanvas(const char* draw_opts)
{
    TString colzopts = draw_opts ? TString(draw_opts) : drawOpts;
    TString coltopts = "col,text";

    hSignalCounter->GetXaxis()->SetTitle(ctx.x.format_string());
    if (dimension::DIM1 == ctx.dim) { hSignalCounter->GetYaxis()->SetTitle(ctx.axis_text.Data()); }
    else if (dimension::DIM2 == ctx.dim)
    {
        hSignalCounter->GetYaxis()->SetTitle(ctx.y.format_string());
        hSignalCounter->GetZaxis()->SetTitle(ctx.axis_text.Data());
    }
    else if (dimension::DIM3 == ctx.dim)
    {
        hSignalCounter->GetYaxis()->SetTitle(ctx.y.format_string());
        hSignalCounter->GetZaxis()->SetTitle(ctx.z.format_string());
        hSignalCounter->SetTitle(ctx.axis_text);
    }

    cSignalCounter->cd();
    hSignalCounter->Draw(colzopts);
    hSignalCounter->SetMarkerColor(kWhite);
    hSignalCounter->SetMarkerSize(1.6);
    hSignalCounter->Sumw2();

    RT::NicePalette((TH2*)hSignalCounter, 0.05);
    RT::NoPalette((TH2*)hSignalCounter);
    gPad->Update();

    // 	float qa_min = 0.;
    // 	float qa_max = 0.;

    // 	float cn_min = 0.;
    // 	float cn_max = 0.;

    // 	for (uint i = 0; i < ctx.cx.bins; ++i)
    // 	{
    // 		if (cSliceXYDiff)
    // 		{
    // 			cSliceXYDiff->cd(1+i);
    // 			hSliceXYDiff[i]->Draw("E");
    // 		}
    //
    // 		if (cSliceXYFitQA)
    // 		{
    // 			cSliceXYFitQA->cd(1+i);
    // 			hSliceXYFitQA[i]->Draw("E");
    //
    // 			for (uint j = 0; j < ctx.cy.bins; ++j)
    // 			{
    // 				float cnt = hSliceXYFitQA[i]->GetBinContent(1+j);
    // 				float err_lo = cnt - hSliceXYFitQA[i]->GetBinErrorLow(1+j);
    // 				float err_up = cnt + hSliceXYFitQA[i]->GetBinErrorUp(1+j);
    //
    // 				if (cnt and (err_lo or err_up))
    // 				{
    // 					if (qa_min == 0.)
    // 						qa_min = err_lo;
    // 					else
    // 						qa_min = err_lo < qa_min ? err_lo : qa_min;
    //
    // 					if (qa_max == 0.)
    // 						qa_max = err_up;
    // 					else
    // 						qa_max = err_up > qa_max ? err_up : qa_max;
    // 				}
    // 			}
    // 		}
    //
    // 		if (cSliceXYChi2NDF)
    // 		{
    // 			cSliceXYChi2NDF->cd(1+i);
    // 			hSliceXYChi2NDF[i]->Draw("E");
    //
    // 			for (uint j = 0; j < ctx.cy.bins; ++j)
    // 			{
    // 				float cnt = hSliceXYChi2NDF[i]->GetBinContent(1+j);
    // 				float err_lo = cnt - hSliceXYChi2NDF[i]->GetBinErrorLow(1+j);
    // 				float err_up = cnt + hSliceXYChi2NDF[i]->GetBinErrorUp(1+j);
    //
    // 				if (cnt and (err_lo or err_up))
    // 				{
    // 					if (cn_min == 0.)
    // 						cn_min = err_lo;
    // 					else
    // 						cn_min = err_lo < cn_min ? err_lo : cn_min;
    //
    // 					if (cn_max == 0.)
    // 						cn_max = err_up;
    // 					else
    // 						cn_max = err_up > cn_max ? err_up : cn_max;
    // 				}
    // 			}
    // 		}
    //
    // 		if (cSliceXYprojX)
    // 		{
    // 			cSliceXYprojX->cd();
    //
    // 			int bmaxx, bmaxy, bmaxz; hSignalCounter->GetMaximumBin(bmaxx, bmaxy, bmaxz);
    // 			int bminx, bminy, bminz; hSignalCounter->GetMinimumBin(bminx, bminy, bminz);
    // 			double max = hSignalCounter->GetBinContent(bmaxx, bmaxy, bmaxz);
    // 			double min = hSignalCounter->GetBinContent(bminx, bminy, bminz);
    // 			double ddelta = (max - min) * 0.1;
    //
    // 			char buff[1000];
    // 			for (uint j = 0; j < ctx.cy.bins; ++j)
    // 			{
    // 				sprintf(buff, "h_%s_xysig_proj_%d", ctx.histPrefix.Data(), j);
    // 				TH1 * h = hSignalCounter->ProjectionX(buff, 1+j, 1+j);
    // 				h->SetLineWidth(1);
    // 				h->SetLineColor(j*4);
    // 				h->SetMarkerStyle(j+20);
    // 				h->SetMarkerColor(j*4);
    //
    // 				if (j == 0)
    // 				{
    // 					h->Draw("hist,E,C");
    // 					h->GetYaxis()->SetRangeUser(min - ddelta, max + ddelta);
    // 				}
    // 				else
    // 				{
    // 					h->Draw("hist,E,C,same");
    // 				}
    // 			}
    // 		}
    // 	}

    // 	float qa_del = 0.05 * (qa_max - qa_min);
    // 	float cn_del = 0.05 * (cn_max - cn_min);
    //
    // 	for (uint i = 0; i < ctx.cx.bins; ++i)
    // 	{
    // 		if (cSliceXYFitQA)
    // 		{
    // 			cSliceXYFitQA->cd(1+i);
    // 			hSliceXYFitQA[i]->GetYaxis()->SetRangeUser(qa_min - qa_del, qa_max + qa_del);
    // 		}
    //
    // 		if (cSliceXYChi2NDF)
    // 		{
    // 			cSliceXYChi2NDF->cd(1+i);
    // 			hSliceXYChi2NDF[i]->GetYaxis()->SetRangeUser(cn_min - cn_del, cn_max + cn_del);
    // 		}
    // 	}
}

// TODO this two should be moved somewhere else, not in library
void DistributionFactory::applyAngDists(double a2, double a4, double corr_a2, double corr_a4)
{
    const size_t hists_num = 1;
    TH1* hist_to_map[hists_num] = {hSignalCounter};

    for (size_t x = 0; x < hists_num; ++x)
        applyAngDists(hist_to_map[x], a2, a4, corr_a2, corr_a4);
}

void DistributionFactory::applyAngDists(TH1* h, double a2, double a4, double corr_a2, double corr_a4)
{
    TF1* f = new TF1("local_legpol", "angdist", -1, 1);
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

    size_t bins_x = h->GetXaxis()->GetNbins();
    size_t bins_y = h->GetYaxis()->GetNbins();

    for (size_t x = 1; x <= bins_x; ++x)
    {
        double bin_l = h->GetXaxis()->GetBinLowEdge(x);
        double bin_r = h->GetXaxis()->GetBinUpEdge(x);
        double bin_w = bin_r - bin_l;
        double corr_factor = 1.0;

        double angmap = f->Integral(bin_l, bin_r);

        if (has_corr) { corr_factor = f_corr->Integral(bin_l, bin_r); }
        else { angmap /= bin_w; }

        double scaling_factor = angmap / corr_factor;
        for (size_t y = 1; y <= bins_y; ++y)
        {
            double tmp_val = h->GetBinContent(x, y);
            h->SetBinContent(x, y, tmp_val * scaling_factor);
        }
    }
    f->Delete();
    if (f_corr) f_corr->Delete();
}

// TODO move away
void DistributionFactory::applyBinomErrors(TH1* N)
{
    const size_t hists_num = 1;
    TH1* hmap[hists_num] = {hSignalCounter};

    for (size_t x = 0; x < hists_num; ++x)
        applyBinomErrors(hmap[x], N);
}

// TODO move away
void DistributionFactory::applyBinomErrors(TH1* q, TH1* N) { RT::calcBinomialErrors(q, N); }

bool copyHistogram(TH1* src, TH1* dst, bool with_functions)
{
    if (!src or !dst) return false;

    size_t bins_x = src->GetXaxis()->GetNbins();
    size_t bins_y = src->GetYaxis()->GetNbins();

    for (size_t x = 1; x <= bins_x; ++x)
    {
        for (size_t y = 1; y <= bins_y; ++y)
        {
            double bc = src->GetBinContent(x, y);
            double be = src->GetBinError(x, y);
            dst->SetBinContent(x, y, bc);
            dst->SetBinError(x, y, be);
        }
    }

    if (!with_functions) return true;

    TList* l = src->GetListOfFunctions();
    if (l->GetEntries())
    {
        dst->GetListOfFunctions()->Clear();
        for (int i = 0; i < l->GetEntries(); ++i)
        {
            TF1* f = (TF1*)l->At(i);
            dst->GetListOfFunctions()->Add(l->At(i)->Clone());
        }
    }

    return true;
}

void DistributionFactory::rename(const char* newname) { Pandora::rename(newname); }

void DistributionFactory::chdir(const char* newdir) { Pandora::chdir(newdir); }

}; // namespace midas

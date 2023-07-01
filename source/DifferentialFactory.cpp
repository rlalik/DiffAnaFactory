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

#include <hellofitty.hpp>

#include <RootTools.h>

#include <TCanvas.h>
#include <TF1.h>
#include <TH1.h>
#include <TLatex.h>
#include <TList.h>

#define PR(x) std::cout << "++DEBUG: " << #x << " = |" << x << "| (" << __FILE__ << ", " << __LINE__ << ")\n";

namespace midas
{

const Option_t h1opts[] = "h,E1";

const TString flags_fit_a = "B,Q,0";
const TString flags_fit_b = "";

DifferentialFactory::DifferentialFactory() : DistributionFactory(), ctx(v_context()), diffs(nullptr)
{
    DistributionFactory::prepare();
    DifferentialFactory::prepare();
}

DifferentialFactory::DifferentialFactory(const v_context& context)
    : DistributionFactory(context), ctx(context), diffs(nullptr)
{
    DistributionFactory::prepare();
    DifferentialFactory::prepare();
}

DifferentialFactory::DifferentialFactory(const v_context* context)
    : DistributionFactory(context), ctx(*context), diffs(nullptr)
{
    DistributionFactory::prepare();
    DifferentialFactory::prepare();
}

DifferentialFactory::~DifferentialFactory()
{
    // 	gSystem->ProcessEvents();
}

DifferentialFactory& DifferentialFactory::operator=(const DifferentialFactory& fa)
{
    if (this == &fa) return *this;

    // 	nthis->objectsFits = new TObjArray();
    // 	nthis->objectsFits->SetName(ctx.name + "Fits");

    (DistributionFactory)(*this) = (DistributionFactory)fa;
    if (!diffs) return *this;

    for (uint i = 0; i < diffs->nhists; ++i)
        copyHistogram((*fa.diffs)[i], (*diffs)[i]);

    return *this;
}

void DifferentialFactory::prepare()
{
    ctx.update();
    objectsFits = new TObjArray();
    objectsFits->SetName(ctx.name + "Fits");
}

void DifferentialFactory::init()
{
    DistributionFactory::init();

    init_diffs();
}

void DifferentialFactory::init_diffs()
{
    diffs =
        new ExtraDimensionMapper(ctx.dim, ctx.name.Data(), hSignalCounter, ctx.v, "@@@d/diffs/%c_@@@a_Signal", this);
}

void DifferentialFactory::reinit()
{
    DistributionFactory::ctx = (context)ctx;
    DistributionFactory::reinit();

    if (diffs)
    {
        diffs->prefix_name = ctx.hist_name;
        diffs->rename(ctx.hist_name);
        diffs->chdir(ctx.dir_name);
    }
}

// void DifferentialFactory::getDiffs(bool with_canvases)
// {
// 	Int_t can_width = 800, can_height = 600;
TString hname, htitle, cname;

// 	if (ctx.useDiff())
// 	{
// 		objectsDiffs = new TObjArray();
// 		objectsDiffs->SetName(ctx.histPrefix + "Diffs");
//
// 		// Lambda: differential plots
// 		hDiscreteXYDiff = new TH1D**[ctx.cx.bins];
// 		if (with_canvases)
// 			c_Diffs = new TCanvas*[ctx.cx.bins];
//
// 		for (uint i = 0; i < ctx.cx.bins; ++i)
// 		{
// 			hDiscreteXYDiff[i] = new TH1D*[ctx.cy.bins];
//
// 			for (uint j = 0; j < ctx.cy.bins; ++j)
// 			{
// 				hname = TString::Format("@@@d/Diffs/h_@@@a_LambdaDiff_%s%02d_%s%02d", "X", i, "Y",
// j); 				htitle = TString::Format(
// 					"#Lambda: %s[%d]=%.1f-%.1f, %s[%d]=%.0f-%.0f;M [MeV/c^{2}];Stat",
// 					ctx.cx.label.Data(), i,
// 					ctx.cx.min+ctx.cx.delta*i,
// 					ctx.cx.min+ctx.cx.delta*(i+1),
// 					ctx.cy.label.Data(), j,
// 					ctx.cy.min+ctx.cy.delta*j,
// 					ctx.cy.min+ctx.cy.delta*(j+1));
//
// 					hDiscreteXYDiff[i][j] = RegTH1<TH1D>(hname, htitle, ctx.v.bins, ctx.v.min,
// ctx.v.max);
//
// 					objectsDiffs->AddLast(hDiscreteXYDiff[i][j]);
// 			}
//
// 			if (with_canvases)
// 			{
// 				cname = TString::Format("@@@d/Diffs/c_@@@a_LambdaDiff_%s%02d", "X", i);
// 				c_Diffs[i] = RegCanvas(cname, "test", can_width, can_height, ctx.cy.bins);
// 			}
// 		}
// 	}
// }

void DifferentialFactory::proceed()
{
    DistributionFactory::proceed();
    if (dimension::DIM3 == ctx.dim)
        diffs->Fill3D(*ctx.x.get_var(), *ctx.y.get_var(), *ctx.z.get_var(), *ctx.v.get_var(), *ctx.var_weight);
    else if (dimension::DIM2 == ctx.dim)
        diffs->Fill2D(*ctx.x.get_var(), *ctx.y.get_var(), *ctx.v.get_var(), *ctx.var_weight);
    else if (dimension::DIM1 == ctx.dim)
        diffs->Fill1D(*ctx.x.get_var(), *ctx.v.get_var(), *ctx.var_weight);
}

void DifferentialFactory::proceed1() { diffs->Fill1D(*ctx.x.get_var(), *ctx.v.get_var(), *ctx.var_weight); }

void DifferentialFactory::proceed2()
{
    diffs->Fill2D(*ctx.x.get_var(), *ctx.y.get_var(), *ctx.v.get_var(), *ctx.var_weight);
}

void DifferentialFactory::proceed3()
{
    diffs->Fill3D(*ctx.x.get_var(), *ctx.y.get_var(), *ctx.z.get_var(), *ctx.v.get_var(), *ctx.var_weight);
}

void DifferentialFactory::binnorm() { DistributionFactory::binnorm(); }

void DifferentialFactory::scale(Float_t factor)
{
    DistributionFactory::scale(factor);

    if (diffs)
    {
        for (uint i = 0; i < diffs->getNHists(); ++i)
        {
            TH1* h = (*diffs)[i];
            if (h) h->Scale(factor);
            // 				if (hSliceXYDiff) hSliceXYDiff[i]->Scale(factor);
        }
    }
}

// void DifferentialFactory::finalize(bool flag_details)
// {
//   DistributionFactory::finalize(flag_details);
// }

// TODO this two should be moved somewhere else, not in library
void DifferentialFactory::applyAngDists(double a2, double a4, double corr_a2, double corr_a4)
{
    DistributionFactory::applyAngDists(a2, a4, corr_a2, corr_a4);
    if (diffs)
    {
        for (uint i = 0; i < diffs->getNHists(); ++i)
        {
            DistributionFactory::applyAngDists((TH1*)(*diffs)[i], a2, a4, corr_a2, corr_a4);
        }
    }
}

// TODO move away
void DifferentialFactory::applyBinomErrors(TH1* N)
{
    DistributionFactory::applyBinomErrors(N);
    // FIXME do it for diffs ?
}

bool DifferentialFactory::write(const char* filename, bool verbose)
{
    return DistributionFactory::write(filename, verbose) && diffs ? diffs->write(filename, verbose) : true;
}

bool DifferentialFactory::write(TFile* f, bool verbose)
{
    return DistributionFactory::write(f, verbose) && diffs ? diffs->write(f, verbose) : true;
}

void DifferentialFactory::niceDiffs(float mt, float mr, float mb, float ml, int ndivx, int ndivy, float xls, float xts,
                                    float xto, float yls, float yts, float yto, bool centerY, bool centerX)
{
    if (diffs)
    {
        for (uint i = 0; i < diffs->getNHists(); ++i)
        {
            UInt_t bx, by, bz;
            diffs->reverseBin(i, bx, by, bz);
            TVirtualPad* p = diffs->getPad(bx, by, bz);
            RT::Hist::NicePad(p, mt, mr, mb, ml);

            TH1* h = (*diffs)[i];
            RT::Hist::NiceHistogram(h, ndivx, ndivy, xls, 0.005, xts, xto, yls, 0.005, yts, yto, centerY, centerX);
        }
    }
}

void DifferentialFactory::niceSlices(float mt, float mr, float mb, float ml, int ndivx, int ndivy, float xls, float xts,
                                     float xto, float yls, float yts, float yto, bool centerY, bool centerX)
{
    // 	for (uint i = 0; i < ctx.cx.bins; ++i)
    // 	{
    // 		TVirtualPad * p = cSliceXYDiff->cd(1+i);
    // 		RT::NicePad(p, mt, mr, mb, ml);
    //
    // 		TH1 * h = hSliceXYDiff[i];
    // 		RT::NiceHistogram(h, ndivx, ndivy, xls, 0.005, xts, xto, yls, 0.005, yts, yto, centerY,
    // centerX);
    // 	}
}

void DifferentialFactory::prepareDiffCanvas()
{
    TLatex* latex = new TLatex();
    latex->SetNDC();
    latex->SetTextSize(0.07);

    TLatex* nflatex = new TLatex();
    nflatex->SetNDC();
    nflatex->SetTextSize(0.07);
    nflatex->SetTextAlign(23);

    size_t nhists = diffs->getNHists();
    for (uint i = 0; i < nhists; ++i)
    {
        UInt_t bx = 0, by = 0, bz = 0;
        diffs->reverseBin(i, bx, by, bz);

        TVirtualPad* pad = diffs->getPad(bx, by, bz);
        pad->cd();
        TH1* h = diffs->get(bx, by, bz);
        h->Draw();
        int pad_number = 0;
        if (ctx.dim == dimension::DIM1)
            pad_number = bx;
        else if (ctx.dim == dimension::DIM2)
            pad_number = by;
        else if (ctx.dim == dimension::DIM3)
            pad_number = bz;

        latex->DrawLatex(0.12, 0.85, TString::Format("%02d", pad_number));

        TList* flist = h->GetListOfFunctions();
        size_t fs = flist->GetEntries();
        if (fs < 3)
        {
            latex->DrawLatex(0.55, 0.85, TString::Format("E=%g", h->GetEntries()));
            latex->DrawLatex(0.55, 0.80, TString::Format("R=%g", h->GetRMS()));
            latex->DrawLatex(0.55, 0.75, TString::Format("E/R=%g", h->GetEntries() / h->GetRMS()));

            nflatex->DrawLatex(0.5, 0.5, "No fit");
            continue;
        }

        Float_t Y_l = ctx.y.get_min() + ctx.y.get_delta() * by;
        Float_t Y_h = ctx.y.get_min() + ctx.y.get_delta() * (by + 1);
        Float_t X_l = ctx.x.get_min() + ctx.x.get_delta() * bx;
        Float_t X_h = ctx.x.get_min() + ctx.x.get_delta() * (bx + 1);

        TF1* tfSum = (TF1*)flist->At(0);
        TF1* tfSig = (TF1*)flist->At(1);
        TF1* tfBkg = (TF1*)flist->At(2);

        tfSig->SetLineColor(kBlack);
        tfSig->SetLineWidth(1);
        tfSig->SetFillColor(kGray);
        tfSig->SetFillStyle(3000);

        tfBkg->SetLineColor(kGray + 2);
        tfBkg->SetLineWidth(1);
        // 	tfBkg->SetLineStyle(7);

        tfSum->SetLineColor(kRed);
        tfSum->SetLineWidth(1);

        tfBkg->Draw("same");
        tfSum->Draw("same");

        TH1* hsigclone = ((TH1*)h->Clone("hsig"));
        hsigclone->Add(tfBkg, -1);
        hsigclone->Delete();

        h->SetTitle("");
        TLatex* latex = new TLatex();
        latex->SetNDC();
        latex->SetTextColor(/*36*/ 1);
        latex->SetTextSize(0.06);
        Int_t oldalign = latex->GetTextAlign();
        Int_t centeralign = 23;

        Float_t centerpos = (1 - pad->GetRightMargin() + pad->GetLeftMargin()) / 2;

        latex->SetTextAlign(centeralign);
        latex->DrawLatex(centerpos, 1.01, TString::Format("%.2f < %s < %.2f", X_l, ctx.x.get_label().Data(), X_h));
        latex->DrawLatex(centerpos, 0.96, TString::Format("%.0f < %s < %.0f", Y_l, ctx.y.get_label().Data(), Y_h));
        latex->SetTextAlign(oldalign);
        latex->SetTextColor(/*36*/ 1);

        int fitnpar = tfSig->GetNpar();
        for (int i = 0; i < fitnpar; ++i)
        {
            latex->DrawLatex(0.5, 0.81 - 0.05 * i,
                             TString::Format("[%d] %5g#pm%.2g", i, tfSig->GetParameter(i), tfSig->GetParError(i)));
        }
        latex->DrawLatex(0.5, 0.25, TString::Format("#chi^{2}/ndf = %g", tfSum->GetChisquare() / tfSum->GetNDF()));
        latex->DrawLatex(0.5, 0.20, TString::Format(" %.2g/%d", tfSum->GetChisquare(), tfSum->GetNDF()));
    }
    latex->Delete();
}

void DifferentialFactory::fitDiffHists(DistributionFactory* sigfac, hf::fitter& hf, hf::fit_entry& stdfit,
                                       bool integral_only)
{
    // 	FitResultData res;
    bool res;

    TLatex* nofit_text = new TLatex();
    nofit_text->SetTextAlign(23);
    nofit_text->SetNDC();

    int info_text = 0;
    TVirtualPad* pad = nullptr;
    TCanvas* can = nullptr;

    UInt_t lx = diffs->nbins_x;
    UInt_t ly = diffs->nbins_y;
    UInt_t lz = diffs->nbins_z;

    for (UInt_t bx = 0; (bx < lx && lx > 0) || bx == 0; ++bx)
        for (UInt_t by = 0; (by < ly && ly > 0) || by == 0; ++by)
            for (UInt_t bz = 0; (bz < lz && lz > 0) || bz == 0; ++bz)
            {
                if (ctx.dim == dimension::DIM3)
                {
                    can = diffs->getCanvas(bx, by);
                    pad = can->cd(bz + 1);
                }
                else if (ctx.dim == dimension::DIM2)
                {
                    can = diffs->getCanvas(bx);
                    pad = can->cd(by + 1);
                }
                else if (ctx.dim == dimension::DIM1)
                {
                    can = diffs->getCanvas(0);
                    pad = can->cd(bx + 1);
                }

                // 		can->Draw(h1opts); FIXME ???
                RT::Hist::NicePad(pad, 0.10, 0.01, 0.15, 0.10);

                TH1D* hfit = diffs->get(bx, by, bz);
                hfit->SetStats(0);
                hfit->Draw();
                info_text = 0;

                if (!integral_only)
                {
                    auto hfp = hf.find_fit(hfit);

                    bool cloned = false;
                    /*                    if (!hfp)
                                        {
                                            cloned = true;
                                            hfp = stdfit.clone(hfit->GetName());
                                            ff.insertParameters(hfp);
                                        }
                    */                    // 				bool hasfunc = ( fflags == FitterFactory::USE_FOUND);
                    bool hasfunc = true;

                    if (((!hasfunc) or
                         (hasfunc and !hfp->get_flag_disabled())) /*and*/ /*(hDiscreteXYDiff[i][j]->GetEntries()
                                                                      > 50)*/
                        /* and (hDiscreteXYDiff[i][j]->GetRMS() < 15)*/)
                    {
                        if ((hfit->GetEntries() / hfit->GetRMS()) < 5)
                        {
                            // 						PR(( hDiscreteXYDiff[i][j]->GetEntries() /
                            // hDiscreteXYDiff[i][j]->GetRMS() ));
                            //						pad->SetFillColor(40);		// FIXME I dont want colors
                            // in the putput
                            info_text = 1;
                        }
                        else
                        {
                            if (!cloned)
                                printf("+ Fitting %s with custom function\n", hfit->GetName());
                            else
                                printf("+ Fitting %s with standard function\n", hfit->GetName());

                            res = fitDiffHist(hfit, hfp);

                            //                            if (res) hfp->update();

                            if (fitCallback) (*fitCallback)(this, sigfac, res, hfit, bx, by, bz);

                            // 						FIXME
                            // 						std::cout << "    Signal: " << res.signal << " +/- "
                            // << res.signal_err << std::endl;

                            // 						hSliceXYDiff[i]->SetBinContent(1+j, res.signal);
                            // 						hSliceXYDiff[i]->SetBinError(1+j,
                            // res.signal_err); 						hSignalCounter->SetBinContent(1+bx, 1+by,
                            // 1+bz, res.signal);
                            // hSignalCounter->SetBinError(1+bx, 1+by, 1+bz, res.signal_err);

                            // 						if (res.mean != 0)
                            // 						{
                            // 							hSliceXYFitQA[i]->SetBinContent(1+j,
                            // res.mean); 							hSliceXYFitQA[i]->SetBinError(1+j,
                            // res.sigma); 							hSliceXYChi2NDF[i]->SetBinContent(1+j,
                            // res.chi2/res.ndf);
                            // 						}
                        }
                    }
                    else
                    {
                        //					pad->SetFillColor(42);
                        info_text = 2;
                    }
                }
                else
                {
                    // 				FIXME
                    // 				res.signal = hDiscreteXYDiff[i][j]->Integral();

                    // 				if (res.signal < 0)  // FIXME old value 500
                    // 				{
                    // 					res.signal = 0;
                    // 				}

                    // 				res.signal_err = RootTools::calcTotalError( hDiscreteXYDiff[i][j],
                    // 1, hDiscreteXYDiff[i][j]->GetNbinsX() );
                    // hSliceXYDiff[i]->SetBinContent(1+j, res.signal);
                    // hSliceXYDiff[i]->SetBinError(1+j, res.signal_err);
                    // 				hDiscreteXYSig->SetBinContent(1+i, 1+j, res.signal);
                    // 				hDiscreteXYSig->SetBinError(1+i, 1+j, res.signal_err);

                    if (fitCallback) (*fitCallback)(this, sigfac, -1, hfit, bx, by, bz);
                }

                Double_t hmax = hfit->GetBinContent(hfit->GetMaximumBin());
                hfit->GetYaxis()->SetRangeUser(0, hmax * 1.1);
                hfit->GetYaxis()->SetNdivisions(504, kTRUE);

                switch (info_text)
                {
                    case 1:
                        nofit_text->DrawLatex(0.65, 0.65, "No fit");
                        break;
                    case 2:
                        nofit_text->DrawLatex(0.65, 0.65, "Fit disabled");
                        break;
                    default:
                        break;
                }

                // 		cSliceXYDiff->cd(1+i)/*->Draw()*/; FIXME
                // 		hSliceXYDiff[i]->Draw(h1opts); FIXME
                //     diffs->getCanvas()
            }

    // 	cDiscreteXYSig->cd(); FIXME
    // 	hDiscreteXYSig->SetMarkerColor(kWhite);

    // 	if (flag_details)
    // 		gStyle->SetPaintTextFormat(".3g");
    // 		hDiscreteXYSig->Draw("colz,text10");
    // 		gStyle->SetPaintTextFormat("g");
    // 	else
    // 		hDiscreteXYSig->Draw("colz");

    if (!sigfac) return;

    RT::NicePalette((TH2*)(sigfac->hSignalCounter), 0.05);

    printf("Raw/fine binning counts:  %f / %f  for %s\n", sigfac->hSignalCounter->Integral(),
           sigfac->hSignalCounter->Integral(), ctx.hist_name.Data());
}

bool DifferentialFactory::fitDiffHist(TH1* hist, hf::fit_entry* hfp, double min_entries)
{
    Int_t bin_l = hist->FindBin(hfp->get_fit_range_min());
    Int_t bin_u = hist->FindBin(hfp->get_fit_range_max());

    // rebin histogram if requested
    if (hfp->get_flag_rebin() != 0) hist->Rebin(hfp->get_flag_rebin());

    // if no data in requested range, nothing to do here
    if (hist->Integral(bin_l, bin_u) == 0) return false;

    // remove all saved function, potentially risky move
    // if has stored other functions than fit functions
    hist->GetListOfFunctions()->Clear();

    // declare functions for fit and signal
    // 	TF1 * tfSum = nullptr;
    TF1* tfSig = nullptr;

    // do fit using FitterFactory
    hf::fitter hf;
    bool res = hf.fit(hfp, hist, "B,Q", "");

    // if fit converged retrieve fit functions from histogram
    // otherwise nothing to do here
    if (!res) return false;

    // 	tfSum = (TF1*)hist->GetListOfFunctions()->At(0);
    tfSig = (TF1*)hist->GetListOfFunctions()->At(1);

    // do not draw Sig function in the histogram
    tfSig->SetBit(TF1::kNotDraw);

    return true;
}

void DifferentialFactory::rename(const char* newname)
{
    DistributionFactory::rename(newname);
    if (diffs) diffs->rename(newname);
}

void DifferentialFactory::chdir(const char* newdir)
{
    DistributionFactory::chdir(newdir);
    if (diffs) diffs->chdir(newdir);
}

void DifferentialFactory::reset()
{
    DistributionFactory::reset();
    if (diffs) diffs->reset();
}

} // namespace midas

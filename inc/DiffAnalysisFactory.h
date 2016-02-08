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


#ifndef DIFFANALYSISFACTORY_H
#define DIFFANALYSISFACTORY_H

#include "TObject.h"
#include "TString.h"
#include "TChain.h"
#include "Rtypes.h"
#include "TDirectory.h"

#include "DiffAnalysisContext.h"
#include "SmartFactory.h"
#include "FitterFactory.h"

class TCanvas;
class TChain;
class TF1;
class TFile;
class TGraph;
class TGraphErrors;
class TH1;
class TH1D;
class TH2;
class TH2D;
class TStyle;
class TVirtualPad;

#ifdef HAVE_HISTASYMMERRORS
#include "TH2DA.h"
#endif

class DiffAnalysisFactory;

typedef void (FitCallback)(DiffAnalysisFactory * fac, int fit_res, TH1 * h, int x_pos, int y_pos);

class DiffAnalysisFactory : public TObject, public SmartFactory {
public:
	DiffAnalysisFactory();
	DiffAnalysisFactory(const DiffAnalysisContext & ctx);
	DiffAnalysisFactory(const DiffAnalysisContext * ctx);
	virtual ~DiffAnalysisFactory();

	DiffAnalysisFactory & operator=(const DiffAnalysisFactory & fa);

	enum Stages { RECO, FIT, SIG, ALL };
	void Init(Stages s = ALL);
	void GetDiffs(bool with_canvases = true);

	void Proceed();
	void Finalize(Stages s = ALL, bool flag_details = false);

	void binnorm();
	void scale(Float_t factor);

	static void niceHisto(TVirtualPad * pad, TH1 * hist, float mt, float mr, float mb, float ml, int ndivx, int ndivy, float xls, float xts, float xto, float yls, float yts, float yto, bool centerY = false, bool centerX = false);

	void niceHists(float mt, float mr, float mb, float ml, int ndivx, int ndivy, float xls, float xts, float xto, float yls, float yts, float yto, bool centerY = false, bool centerX = false);
	void niceDiffs(float mt, float mr, float mb, float ml, int ndivx, int ndivy, float xls, float xts, float xto, float yls, float yts, float yto, bool centerY = false, bool centerX = false);
	void niceSlices(float mt, float mr, float mb, float ml, int ndivx, int ndivy, float xls, float xts, float xto, float yls, float yts, float yto, bool centerY = false, bool centerX = false);

	void fitDiffHists(FitterFactory & ff, HistFitParams & stdfit, bool integral_only = false);
	bool fitDiffHist(TH1 * hist, HistFitParams & hfp, double min_entries = 0);

	const char * GetName() const { return ("Factory"/* + ctx.histPrefix*/); }

	void prepareDiffCanvas();
	void prepareSigCanvas(bool flag_details = false);

	void applyAngDists(double a2, double a4, double corr_a2 = 0.0, double corr_a4 = 0.0);
	static void applyAngDists(TH2 * h, double a2, double a4, double corr_a2 = 0.0, double corr_a4 = 0.0);

	void applyBinomErrors(TH2 * N);
	static void applyBinomErrors(TH2 * q, TH2 * N);

	TH2 ** getSigsArray(size_t & size);

	inline void setFitCallback(FitCallback * cb) { fitCallback = cb; }

private:
	void prepare();
	bool copyHistogram(TH1 * src, TH1 * dst);

public:
	DiffAnalysisContext ctx;		//||

	TH2D * hSignalXY;				//->	// X-Y spectrum
	TCanvas * cSignalXY;			//->

	TH2D * hSignalWithCutsXY;		//->	// X-Y spectrum with cuts
	TCanvas * cSignalWithCutsXY;	//->

	TH2D * hDiscreteXY;				//->	// discretre X-Y
	TCanvas * cDiscreteXY;			//->
	TCanvas * cDiscreteXYFull;		//->	// discrete X-Y on top of regular

#ifdef HAVE_HISTASYMMERRORS
	TH2DA * hDiscreteXYSig;
#else
	TH2D * hDiscreteXYSig;			//->	// discrete X-Y, signal extracted
#endif
	TCanvas * cDiscreteXYSig;		//->
	TCanvas * cDiscreteXYSigFull;	//->

	TH1D *** hDiscreteXYDiff;		//[10]	// 3rd var distribution in diff bin
	TCanvas ** cDiscreteXYDiff;		//!

	TH1D ** hSliceXYFitQA;			//[10]	// QA values
	TCanvas * cSliceXYFitQA;		//!

	TH1D ** hSliceXYChi2NDF;		//[10]	// Chi2/NDF values
	TCanvas * cSliceXYChi2NDF;		//!
	TCanvas * cSliceXYprojX;		//!

	TH1D ** hSliceXYDiff;			//!		// slice of x-var in discrete X-Y
	TCanvas * cSliceXYDiff;			//!

	TObjArray * objectsDiffs;		//!
	TObjArray * objectsSlices;		//!
	TObjArray * objectsFits;		//!

	ClassDef(DiffAnalysisFactory, 1);

private:
	FitCallback * fitCallback;
};

#endif // DIFFANALYSISFACTORY_H
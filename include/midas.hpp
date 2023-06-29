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

#ifndef MIDAS_HPP
#define MIDAS_HPP

#include "Midas/midas_export.hpp"

#include <Pandora.h>

#include <RootTools.h>

#include <TNamed.h>
#include <TString.h>

class TH1;
class TH1D;
class TH2;
class TH2D;
class TVirtualPad;

namespace hf
{
class fit_entry;
class fitter;
}; // namespace hf

namespace midas
{

class MIDAS_EXPORT axis_config final
{
public:
    axis_config();
    axis_config(Float_t* var) : var(var) {}

    auto operator==(const axis_config& ctx) -> bool;
    auto operator!=(const axis_config& ctx) -> bool;

    auto set_variable(Float_t* var) -> axis_config&
    {
        this->var = var;
        return *this;
    }
    auto set_bins(UInt_t bins, Float_t min, Float_t max) -> axis_config&
    {
        this->bins = bins;
        this->min = min;
        this->max = max;
        this->bins_arr = nullptr;
        this->delta = (max - min) / bins;
        return *this;
    }
    auto set_bins(Float_t* bins) -> axis_config&
    {
        this->bins = 0;
        this->min = 0.0;
        this->max = 0.0;
        this->bins_arr = bins;
        this->delta = 0.0;
        return *this;
    }
    auto set_label(TString label) -> axis_config&
    {
        this->label = std::move(label);
        return *this;
    }
    auto set_unit(TString unit) -> axis_config&
    {
        this->unit = std::move(unit);
        return *this;
    }

    auto get_var() const -> const Float_t* { return var; }
    auto get_bins() const -> UInt_t { return bins; }
    auto get_min() const -> Float_t { return min; }
    auto get_max() const -> Float_t { return max; }
    auto get_delta() const -> Float_t { return delta; }
    auto get_bins_array() const -> const Float_t* { return bins_arr; }
    auto get_label() const -> const TString& { return label; }
    auto get_unit() const -> const TString& { return unit; }

    auto format_unit() const -> TString;
    auto format_string() const -> TString;
    auto format_hist_string(const char* title = nullptr, const char* ylabel = "Counts") const -> TString;

    static auto format_unit(const char* unit) -> TString;
    static auto format_unit(const TString& unit) -> TString;

    auto print() const -> void;

private:
    TString label; // label for the axis
    TString unit;  // unit for the axis
    // TString title;     // title for the axis
    Float_t* var;      //!	here is the address of the variable which is used to fill data
    UInt_t bins;       // number of bins
    Float_t min;       // minimum axis value
    Float_t max;       // maximum axis value
    Float_t* bins_arr; //! here one can put custom bins division array
    Float_t delta;     // CAUTION: overriden by validate(), do not set by hand

    // ClassDef(axis_config, 1);
};

enum dimension
{
    NOINIT,
    DIM0,
    DIM1,
    DIM2,
    DIM3
};

class MIDAS_EXPORT context : public TNamed
{
public:
    dimension dim; // define dimension
    // config
    mutable TString name; // prefix for histograms
    TString dir_name;
    TString hist_name; // name for histograms
    TString diff_var_name;

    TString title;
    TString label;
    TString unit;
    TString axis_text;

    axis_config x, y, z; // x, y are two dimensions, V is a final Variable axis

    // cut range when useCut==kTRUE
    // 	Double_t cutMin;			// Cut: min
    // 	Double_t cutMax;			// Cut: max

    // variables to use for diff analysis
    Float_t* var_weight; //!
    // variable used for cuts when cutCut==kTRUE

    context();
    context(dimension dim);
    context(const context& ctx);
    virtual ~context();

    context& operator=(const context& ctx);
    bool operator==(const context& ctx);
    bool operator!=(const context& ctx);

    virtual bool update();
    virtual int validate() const;

    virtual void format_diff_axis();
    virtual TString format_hist_axes(const char* title = nullptr) const;

    virtual const char* AnaName() const { return name.Data(); }

    virtual bool findJsonFile(const char* initial_path, const char* filename, int search_depth = -1);
    virtual bool configureFromJson(const char* name);
    virtual bool configureToJson(const char* name, const char* jsonfile);

    void print() const;

protected:
    TString json_fn;
    bool json_found;

    // ClassDef(distribution_context, 1);
};

class MIDAS_EXPORT v_context : public context
{
public:
    axis_config V; // x, y are two dimensions, V is a final Variable axis

    v_context();
    v_context(const v_context& ctx);
    virtual ~v_context();

    v_context& operator=(const v_context& ctx);
    bool operator==(const v_context& ctx);
    bool operator!=(const v_context& ctx);

    // flags
    // 	virtual bool useCuts() const { return (cutMin or cutMax); }

    virtual bool configureFromJson(const char* name);
    virtual bool configureToJson(const char* name, const char* jsonfile);

    void print() const;

private:
    TString json_fn;
    // ClassDef(DifferentialContext, 2);
};

class MIDAS_EXPORT DistributionFactory : public TObject, public RT::Pandora
{
public:
    DistributionFactory();
    DistributionFactory(const context& ctx);
    DistributionFactory(const context* ctx);
    virtual ~DistributionFactory();

    DistributionFactory& operator=(const DistributionFactory& fa);

    virtual void init();
    virtual void reinit();
    virtual void proceed();
    virtual void finalize(const char* draw_opts = nullptr);

    virtual void binnorm();
    virtual void scale(Float_t factor);

    static void niceHisto(TVirtualPad* pad, TH1* hist, float mt, float mr, float mb, float ml, int ndivx, int ndivy,
                          float xls, float xts, float xto, float yls, float yts, float yto, bool centerY = false,
                          bool centerX = false);

    // 	void niceHists(float mt, float mr, float mb, float ml, int ndivx, int ndivy, float xls,
    // float xts, float xto, float yls, float yts, float yto, bool centerY = false, bool centerX =
    // false);
    virtual void niceHists(RT::Hist::PadFormat pf, const RT::Hist::GraphFormat& format);

    virtual void prepareCanvas(const char* draw_opts = nullptr);

    virtual void applyAngDists(double a2, double a4, double corr_a2 = 0.0, double corr_a4 = 0.0);
    static void applyAngDists(TH1* h, double a2, double a4, double corr_a2 = 0.0, double corr_a4 = 0.0);

    virtual void applyBinomErrors(TH1* N);
    static void applyBinomErrors(TH1* q, TH1* N);

    void setDrawOptions(const char* draw_opts) { drawOpts = draw_opts; }

protected:
    virtual void prepare();
    virtual void rename(const char* newname);
    virtual void chdir(const char* newdir);

public:
    context ctx; //||

    // #ifdef HAVE_HISTASYMMERRORS
    // 	TH2DA * hSignalCounter;
    // #else
    // 	TH2D * hSignalCounter;			//->	// discrete X-Y, signal extracted
    // #endif
    TH1* hSignalCounter;     //->	// discrete X-Y, signal extracted
    TCanvas* cSignalCounter; //->

    // 	TCanvas * cDiscreteXYSig;		//->
    // 	TCanvas * cDiscreteXYSigFull;	//->

protected:
    TString drawOpts;

    // ClassDef(DistributionFactory, 1);
};

class MIDAS_EXPORT ExtraDimensionMapper : public TObject, public RT::Pandora
{
public:
    ExtraDimensionMapper(dimension dim, const std::string& name, TH1* hist, const axis_config& axis,
                         const std::string& dir_and_name);
    ExtraDimensionMapper(dimension dim, const std::string& name, TH1* hist, const axis_config& axis,
                         const std::string& dir_and_name, RT::Pandora* sf);
    virtual ~ExtraDimensionMapper();

    UInt_t getBinsX() const { return nbins_x; }
    UInt_t getBinsY() const { return nbins_y; }
    UInt_t getBinsZ() const { return nbins_z; }

    UInt_t getBin(UInt_t x, UInt_t y = 0, UInt_t z = 0) const;
    bool reverseBin(UInt_t bin, UInt_t& x) const;
    bool reverseBin(UInt_t bin, UInt_t& x, UInt_t& y) const;
    bool reverseBin(UInt_t bin, UInt_t& x, UInt_t& y, UInt_t& z) const;

    TH1D* get(UInt_t x, UInt_t y = 0, UInt_t z = 0);
    TH1D* find(Double_t x, Double_t y = 0.0, Double_t z = 0.0);
    TCanvas* getCanvas(UInt_t x, UInt_t y = 0);
    TVirtualPad* getPad(UInt_t x, UInt_t y = 0, UInt_t z = 0);

    size_t getNHists() const { return nhists; }
    TH1D* operator[](int n) { return histograms[n]; }
    const TH1D* operator[](int n) const { return histograms[n]; }

    void Fill1D(Float_t x, Float_t v, Float_t w = 1.0);
    void Fill2D(Float_t x, Float_t y, Float_t v, Float_t w = 1.0);
    void Fill3D(Float_t x, Float_t y, Float_t z, Float_t v, Float_t w = 1.0);

    // ExtraDimensionMapper & operator=(const ExtraDimensionMapper & fa);
private:
    void map1D(const axis_config& axis);
    void map2D(const axis_config& axis);
    void map3D(const axis_config& axis);
    void formatName(char* buff, UInt_t x, UInt_t y = 0, UInt_t z = 0);
    void formatCanvasName(char* buff, UInt_t x, UInt_t y = 0);

public:
    dimension dim;
    axis_config axis; //||
    std::string prefix_name;

    UInt_t nhists;
    UInt_t nbins_x, nbins_y, nbins_z;

    TH1* ref_hist;
    TH1D** histograms;  //!
    TCanvas** canvases; //!

    // 	TObjArray * objectsFits;

    // ClassDef(ExtraDimensionMapper, 1);

private:
    // 	FitCallback * fitCallback;
};

class DifferentialFactory;

typedef void(FitCallback)(DifferentialFactory* fac, DistributionFactory* sigfac, int fit_res, TH1* h, uint x_pos,
                          uint y_pos, uint z_pos);

class MIDAS_EXPORT DifferentialFactory : public DistributionFactory
{
public:
    DifferentialFactory();
    DifferentialFactory(const v_context& ctx);
    DifferentialFactory(const v_context* ctx);
    virtual ~DifferentialFactory();

    DifferentialFactory& operator=(const DifferentialFactory& fa);

    // 	void getDiffs(bool with_canvases = true);

    virtual void prepare();
    virtual void init();
    virtual void reinit();
    virtual void proceed();
    // 	virtual void finalize(bool flag_details = false);

    virtual void reset();

    virtual void binnorm();
    virtual void scale(Float_t factor);

    virtual void applyAngDists(double a2, double a4, double corr_a2 = 0.0, double corr_a4 = 0.0);

    virtual void applyBinomErrors(TH1* N);

    bool write(TFile* f /* = nullptr*/, bool verbose = false);
    bool write(const char* filename /* = nullptr*/, bool verbose = false);

    void niceDiffs(float mt, float mr, float mb, float ml, int ndivx, int ndivy, float xls, float xts, float xto,
                   float yls, float yts, float yto, bool centerY = false, bool centerX = false);
    void niceSlices(float mt, float mr, float mb, float ml, int ndivx, int ndivy, float xls, float xts, float xto,
                    float yls, float yts, float yto, bool centerY = false, bool centerX = false);

    void fitDiffHists(DistributionFactory* sigfac, hf::fitter& hf, hf::fit_entry& stdfit, bool integral_only = false);
    bool fitDiffHist(TH1* hist, hf::fit_entry* hfp, double min_entries = 0);

    void setFitCallback(FitCallback* cb) { fitCallback = cb; }
    virtual void prepareDiffCanvas();

protected:
    virtual void rename(const char* newname);
    virtual void chdir(const char* newdir);

private:
    virtual void init_diffs();
    virtual void proceed1();
    virtual void proceed2();
    virtual void proceed3();

public:
    v_context ctx;
    ExtraDimensionMapper* diffs;
    TCanvas** c_Diffs;      //!
    TObjArray* objectsFits; //!

private:
    FitCallback* fitCallback;

    // ClassDef(DifferentialFactory, 1);
};

bool copyHistogram(TH1* src, TH1* dst, bool with_functions = true);

}; // namespace midas

#endif // MIDAS_HPP

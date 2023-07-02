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

#include <TF1.h>
#include <TNamed.h>
#include <TString.h>

#include <functional>
#include <stdexcept>

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

// Thrown when the dimension is incorrect
class dimension_error : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

/// Stores pointer to a variable and histogram description in means of bins, range
/// (or array of bins) and also axis description like label and unit.
class MIDAS_EXPORT axis_config final
{
public:
    axis_config();
    /// @param ptr the variable pointer
    axis_config(Float_t* ptr) : var(ptr) {}

    auto operator==(const axis_config& ctx) -> bool;
    auto operator!=(const axis_config& ctx) -> bool;

    auto set_variable(Float_t* ptr) -> axis_config&
    {
        var = ptr;
        return *this;
    }
    auto set_bins(Int_t hist_bins, Float_t range_min, Float_t range_max) -> axis_config&
    {
        bins = hist_bins;
        min = range_min;
        max = range_max;
        bins_arr = nullptr;
        delta = (max - min) / static_cast<Float_t>(bins);
        return *this;
    }
    auto set_bins(Float_t* bins_array) -> axis_config&
    {
        bins = 0;
        min = 0.0;
        max = 0.0;
        bins_arr = bins_array;
        delta = 0.0;
        return *this;
    }
    auto set_label(TString axis_label) -> axis_config&
    {
        label = std::move(axis_label);
        return *this;
    }
    auto set_unit(TString axis_unit) -> axis_config&
    {
        unit = std::move(axis_unit);
        return *this;
    }

    auto get_var() const -> const Float_t* { return var; }
    auto get_bins() const -> Int_t { return bins; }
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
    // TString title;   // title for the axis
    Float_t* var;      //!	here is the address of the variable which is used to fill data
    Int_t bins;        // number of bins
    Float_t min;       // minimum axis value
    Float_t max;       // maximum axis value
    Float_t* bins_arr; //! here one can put custom bins division array
    Float_t delta;     //! CAUTION: overriden by validate(), do not set by hand

    // ClassDef(axis_config, 1);
};

enum class dimension
{
    NODIM,
    DIM1,
    DIM2,
    DIM3
};

class basic_distribution;
class context;
class distribution;
class observable;

class MIDAS_EXPORT basic_context : public TNamed
{
protected:
    dimension dim; // define dimension
    // config
    mutable TString name; // prefix for histograms
    TString dir_name;
    TString hist_name; // name for histograms
    TString diff_var_name;

    TString context_title;
    TString context_label;
    TString context_unit;
    TString context_axis_text;

    axis_config x, y, z; // x..z are dimensions, V is an observable variable axis

    // 	Double_t cutMin;			// Cut: min
    // 	Double_t cutMax;			// Cut: max

    // variables to use for diff analysis
    Float_t* var_weight; //!
    // variable used for cuts when cutCut==kTRUE

protected:
    basic_context();

public:
    basic_context(TString context_name, dimension context_dim);
    basic_context(TString context_name, axis_config x_axis);
    basic_context(TString context_name, axis_config x_axis, axis_config y_axis);
    basic_context(TString context_name, axis_config x_axis, axis_config y_axis, axis_config z_axis);
    basic_context(const basic_context&) = default;
    basic_context(basic_context&&) = default;
    virtual ~basic_context() = default;

    basic_context& operator=(const basic_context&) = default;
    basic_context& operator=(basic_context&&) = default;

    bool operator==(const basic_context& ctx);
    bool operator!=(const basic_context& ctx);

    auto cast(TString new_name, dimension new_dim) const -> basic_context;
    auto reduce() -> void;
    auto extend() -> axis_config&;
    auto extend(axis_config next_dim) -> void;

    auto get_x() -> axis_config&;
    auto get_y() -> axis_config&;
    auto get_z() -> axis_config&;

    [[nodiscard]] auto expand(axis_config extra_dim) -> context;

    // auto get_dimension() const -> dimension { return dim; }
    auto get_name() const -> const TString { return name; }
    auto get_title() const -> const TString { return context_title; }
    auto get_label() const -> const TString& { return context_label; }
    // auto get_unit() const -> const TString& { return unit; }
    // auto get_axis_text() const -> const TString& { return axis_text; }

    virtual bool update();
    virtual int validate() const;

    virtual void format_diff_axis();
    virtual TString format_hist_axes(const char* title = nullptr) const;

    virtual bool findJsonFile(const char* initial_path, const char* filename, int search_depth = -1);
    virtual bool configureFromJson(const char* name);
    virtual bool configureToJson(const char* name, const char* jsonfile);

    void print() const;

protected:
    TString json_fn;
    bool json_found;

    friend basic_distribution;
    friend distribution;

    // ClassDef(distribution_context, 1);
};

class MIDAS_EXPORT context : public basic_context
{
protected:
    axis_config v; // x, y are two dimensions, V is a final Variable axis

private:
    context();

public:
    using basic_context::basic_context;
    context(TString name, dimension dim, axis_config v_axis);
    context(TString name, axis_config x_axis, axis_config v_axis);
    context(TString name, axis_config x_axis, axis_config y_axis, axis_config v_axis);
    context(TString name, axis_config x_axis, axis_config y_axis, axis_config z_axis, axis_config v_axis);
    context(const basic_context& ctx);
    context(const context& ctx);
    virtual ~context();

    context& operator=(const basic_context& ctx);
    context& operator=(const context& ctx);
    bool operator==(const context& ctx);
    bool operator!=(const context& ctx);

    auto get_v() -> axis_config& { return v; }
    auto get_v() const -> const axis_config& { return v; }

    // flags
    // 	virtual bool useCuts() const { return (cutMin or cutMax); }

    virtual bool configureFromJson(const char* name);
    virtual bool configureToJson(const char* name, const char* jsonfile);

    void print() const;

private:
    TString json_fn;

    friend distribution;
    friend observable;

    // ClassDef(DifferentialContext, 2);
};

class MIDAS_EXPORT basic_distribution : public TObject, public RT::Pandora
{
protected:
    basic_distribution();

public:
    basic_distribution(const basic_context& ctx);
    basic_distribution(const basic_context* ctx);
    basic_distribution(const basic_distribution&) = delete;
    basic_distribution(basic_distribution&&) = default;
    virtual ~basic_distribution();

    basic_distribution& operator=(const basic_distribution& fa) = delete;
    basic_distribution& operator=(basic_distribution&&) = default;

    virtual void reinit();
    virtual void proceed();
    virtual void finalize(const char* draw_opts = nullptr);

    auto transform(std::function<void(TH1* h)> transform_function) -> void;
    auto transform(std::function<void(TCanvas* h)> transform_function) -> void;

    virtual void prepareCanvas(const char* draw_opts = nullptr);

    virtual void applyAngDists(double a2, double a4, double corr_a2 = 0.0, double corr_a4 = 0.0);
    static void applyAngDists(TH1* h, double a2, double a4, double corr_a2 = 0.0, double corr_a4 = 0.0);

    virtual void applyBinomErrors(TH1* N);
    static void applyBinomErrors(TH1* q, TH1* N);

    void setDrawOptions(const char* draw_opts) { drawOpts = draw_opts; }

    auto print() const -> void;

protected:
    virtual void prepare();
    virtual void init();

    virtual void rename(const char* newname);
    virtual void chdir(const char* newdir);

public:
    basic_context ctx; //||

    // #ifdef HAVE_HISTASYMMERRORS
    // 	TH2DA * hSignalCounter;
    // #else
    // 	TH2D * hSignalCounter;			//->	// discrete X-Y, signal extracted
    // #endif
    std::unique_ptr<TH1> hSignalCounter;     //->	// discrete X-Y, signal extracted
    std::unique_ptr<TCanvas> cSignalCounter; //->

    // 	TCanvas * cDiscreteXYSig;		//->
    // 	TCanvas * cDiscreteXYSigFull;	//->

protected:
    TString drawOpts;

    // ClassDef(DistributionFactory, 1);
};

class distribution;

typedef void(FitCallback)(distribution* fac, basic_distribution* sigfac, int fit_res, TH1* h, int x_pos, int y_pos,
                          int z_pos);

class MIDAS_EXPORT distribution : public basic_distribution
{
public:
    distribution();
    distribution(const context& ctx);
    distribution(const context* ctx);
    distribution(const distribution&) = delete;
    distribution(distribution&&) = delete;
    virtual ~distribution();

    distribution& operator=(const distribution& fa) = delete;
    distribution& operator=(distribution&& fa) = delete;

    // 	void getDiffs(bool with_canvases = true);

    virtual void prepare();
    virtual void init();
    virtual void reinit();
    virtual void proceed();
    // 	virtual void finalize(bool flag_details = false);

    virtual void reset();

    // virtual void scale(Float_t factor);

    virtual void applyAngDists(double a2, double a4, double corr_a2 = 0.0, double corr_a4 = 0.0);

    virtual void applyBinomErrors(TH1* N);

    bool write(TFile* f /* = nullptr*/, bool verbose = false);
    bool write(const char* filename /* = nullptr*/, bool verbose = false);

    void niceDiffs(float mt, float mr, float mb, float ml, int ndivx, int ndivy, float xls, float xts, float xto,
                   float yls, float yts, float yto, bool centerY = false, bool centerX = false);
    void niceSlices(float mt, float mr, float mb, float ml, int ndivx, int ndivy, float xls, float xts, float xto,
                    float yls, float yts, float yto, bool centerY = false, bool centerX = false);

    void fitDiffHists(basic_distribution* sigfac, hf::fitter& hf, hf::fit_entry& stdfit, bool integral_only = false);
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
    context ctx;
    observable* diffs;
    TCanvas** c_Diffs;      //!
    TObjArray* objectsFits; //!

private:
    FitCallback* fitCallback;

    // ClassDef(DifferentialFactory, 1);
};

template <class T> auto scale(T* fac, Float_t factor) -> void
{
    fac->transform([&](TH1* h) { h->Scale(factor); });
}

template <class T> auto bin_normalization(T* fac) -> void
{
    fac->transform([](TH1* h) { h->Scale(1.0 / (h->GetXaxis()->GetBinWidth(1) * h->GetYaxis()->GetBinWidth(1))); });
}

template <class T> auto apply_ang_distribution(T* fac, double a2, double a4, double corr_a2, double corr_a4) -> void
{
    fac->transform(
        [&](TH1* h)
        {
            auto f = std::unique_ptr<TF1>(new TF1("local_legpol", "angdist", -1, 1));
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
            if (f_corr) f_corr->Delete();
        });
}

}; // namespace midas

#endif // MIDAS_HPP

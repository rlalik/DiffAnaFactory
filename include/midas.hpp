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

#include <pandora.hpp>

#include <TCanvas.h>
#include <TF1.h>
#include <TH1.h>
#include <TNamed.h>
#include <TString.h>

#include <functional>
#include <stdexcept>

class TH2;
class TH2D;
class TVirtualPad;
class TFile;

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
class MIDAS_EXPORT axis_config final : public TObject
{
public:
    axis_config();
    /// @param ptr the variable pointer
    axis_config(Float_t* ptr) : var(ptr) {}
    axis_config(const axis_config&) = default;
    axis_config(axis_config&&) = default;
    virtual ~axis_config() = default;

    auto operator=(const axis_config&) -> axis_config& = default;
    auto operator=(axis_config&&) -> axis_config& = default;

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
    template <size_t size> auto set_bins(Double_t (&bins_array)[size]) -> axis_config&
    {
        bins = size - 1;
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
    constexpr auto get_bins() const -> Int_t { return bins; }
    constexpr auto get_min() const -> Float_t { return min; }
    constexpr auto get_max() const -> Float_t { return max; }
    auto get_delta() const -> Float_t { return delta; }
    constexpr auto get_bins_array() const -> const Double_t* { return bins_arr; }
    auto get_label() const -> const TString& { return label; }
    auto get_unit() const -> const TString& { return unit; }

    auto format_unit() const -> TString;
    auto format_string() const -> TString;
    auto format_hist_string(const char* title = nullptr, const char* ylabel = "Counts") const -> TString;

    static auto format_unit(const char* unit) -> TString;
    static auto format_unit(const TString& unit) -> TString;

    auto print() const -> void;
    auto validate() const -> bool;

private:
    TString label; // label for the axis
    TString unit;  // unit for the axis
    // TString title;   // title for the axis
    Float_t* var;       //!	here is the address of the variable which is used to fill data
    Int_t bins;         // number of bins
    Float_t min;        // minimum axis value
    Float_t max;        // maximum axis value
    Double_t* bins_arr; //! here one can put custom bins division array
    Float_t delta;      //! CAUTION: overridden by validate(), do not set by hand

    ClassDef(axis_config, 1)
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

/// Basic context to store primitive distribution. It is intended to be a base class for @see midas::context context for
/// extended distribution.
class MIDAS_EXPORT basic_context : public TNamed
{
protected:
public:
    basic_context();
    /// Initialize context with name and the dimension
    /// @param context_name the context name
    /// @param context_dim the context dimension
    basic_context(TString context_name, dimension context_dim);
    /// Initialzie 1D context with name
    /// @param context_name the context name
    /// @param x_axis the x axis description
    basic_context(TString context_name, axis_config x_axis);
    /// Initialzie 2D context with name
    /// @param context_name the context name
    /// @param x_axis the x axis description
    /// @param y_axis the y axis description
    basic_context(TString context_name, axis_config x_axis, axis_config y_axis);
    /// Initialzie 1D context with name
    /// @param context_name the context name
    /// @param x_axis the x axis description
    /// @param y_axis the y axis description
    /// @param z_axis the z axis description
    basic_context(TString context_name, axis_config x_axis, axis_config y_axis, axis_config z_axis);
    /// Default copy constructor
    basic_context(const basic_context&) = default;
    /// Default move constructor
    basic_context(basic_context&&) = default;
    /// Default destructor
    virtual ~basic_context() = default;

    /// Default assignment operator
    auto operator=(const basic_context&) -> basic_context& = default;
    /// Default move-assignment operator
    auto operator=(basic_context&&) -> basic_context& = default;

    /// Equality operator
    auto operator==(const basic_context& ctx) -> bool;
    /// Non-equality operator
    auto operator!=(const basic_context& ctx) -> bool;

    /// Cast current context to new dimension, either of lwoer or higher order.
    /// @param new_name new context name
    /// @param new_dim new context dimension
    /// @return new context of requested dimension
    auto cast(TString new_name, dimension new_dim) const -> basic_context;
    /// Reduce dimension by 1
    auto reduce() -> void;
    /// Extend dimension by 1
    /// @return the new dimension axis, to be used for configuring the dimension
    auto extend() -> axis_config&;
    /// Extend dimension by 1 with the given axis description
    /// @param next_dim the axis configuration for the next dimension
    auto extend(axis_config next_dim) -> void;

    /// @{
    /// @name Get axis functions
    /// Get the x,y,z dimension axes
    /// @return the requested axis config
    /// @throw midas::dimension_error if dimension not specified yet
    auto get_x() -> axis_config&;
    auto get_y() -> axis_config&;
    auto get_z() -> axis_config&;
    /// @}

    [[nodiscard]] auto expand(axis_config extra_dim) -> context;

    // auto get_dimension() const -> dimension { return dim; }
    auto get_name() const -> const TString { return name; }
    auto get_title() const -> const TString { return context_title; }
    auto get_label() const -> const TString& { return context_label; }
    // auto get_unit() const -> const TString& { return unit; }
    // auto get_axis_text() const -> const TString& { return axis_text; }

    virtual TString format_hist_axes(const char* title = nullptr) const;

    // virtual bool configureFromJson(const char* name);
    // virtual bool configureToJson(const char* name, const char* jsonfile);

    virtual auto prepare() -> void;

    virtual auto print() const -> void;
    virtual auto validate() const -> bool;

protected:
    dimension dim;        // context dimension
    mutable TString name; // context name
    TString dir_name;     // histograms directory
    TString hist_name;    // histograms common name
    TString diff_label;   // differential distribution variable name, e.g. d^2N/dxdy, etc...

    TString context_title;
    TString context_label;
    TString context_unit;
    TString context_axis_text;

    axis_config x, y, z; // x..z are dimensions, V is an observable variable axis

    TString json_fn;
    bool json_found;

    friend basic_distribution;
    friend distribution;

    ClassDef(basic_context, 1)
};

/// Advanced context which supports also v-axis, and axis for observable which can add additional constrains on the
/// distribution. An example is to create a invariant mass spectrum of some particle to extract the signal in given bin.
class MIDAS_EXPORT context : public basic_context
{
private:
public:
    context();
    context(TString name, dimension dim);
    context(TString name, dimension dim, axis_config v_axis);
    context(TString name, axis_config x_axis, axis_config v_axis);
    context(TString name, axis_config x_axis, axis_config y_axis, axis_config v_axis);
    context(TString name, axis_config x_axis, axis_config y_axis, axis_config z_axis, axis_config v_axis);
    context(const basic_context& ctx);
    context(const context& ctx);
    virtual ~context();

    auto cast(TString new_name, dimension new_dim) const -> context;
    auto expand(axis_config extra_dim) -> context = delete;

    auto get_v() -> axis_config& { return v; }
    auto get_v() const -> const axis_config& { return v; }

    context& operator=(const basic_context& ctx);
    context& operator=(const context& ctx);
    bool operator==(const context& ctx);
    bool operator!=(const context& ctx);

    virtual auto prepare() -> void override;

    virtual auto print() const -> void override;
    virtual auto validate() const -> bool override;

    // virtual bool configureFromJson(const char* name);
    // virtual bool configureToJson(const char* name, const char* jsonfile);

private:
    axis_config v; // x, y are two dimensions, V is a final Variable axis

    TString json_fn;

    friend distribution;
    friend observable;

    ClassDefOverride(context, 1)
};

class MIDAS_EXPORT basic_distribution : public TObject
{
protected:
    // basic_distribution();

public:
    basic_distribution(const basic_context& ctx, pandora::pandora* pbox = nullptr);
    basic_distribution(const basic_distribution&) = delete;
    basic_distribution(basic_distribution&&) = default;
    virtual ~basic_distribution();

    auto operator=(const basic_distribution& fa) -> basic_distribution& = delete;
    auto operator=(basic_distribution&&) -> basic_distribution& = default;

    auto get_signal_hist() -> TH1* { return signal_histogram.get(); }

    virtual auto fill(Float_t weight = 1.0) -> void;
    virtual auto prepare() -> void;
    virtual auto reinit() -> void;
    virtual auto finalize(const char* draw_opts = nullptr) -> void;

    virtual auto validate() const -> bool;
    virtual auto print() const -> void;

    auto transform(std::function<void(TH1* h)> transform_function) -> void;
    auto transform(std::function<void(TCanvas* h)> transform_function) -> void;

    virtual auto prepare_canvas(const char* draw_opts = nullptr) -> void;

    virtual auto applyBinomErrors(TH1* N) -> void;
    static auto applyBinomErrors(TH1* q, TH1* N) -> void;

    auto set_draw_options(const char* draw_opts) -> void { drawOpts = draw_opts; }

    virtual auto save(TFile* f /* = nullptr*/, bool verbose = false) -> bool;
    virtual auto save(const char* filename /* = nullptr*/, bool verbose = false) -> bool;

    template <typename T, typename... Types> T* reg_hist(const char* name, const char* title, Types... arguments)
    {
        if (box)
            return box->reg_hist<T>(name, title, arguments...);
        else
        {
            auto h = new T(name, title, arguments...);
            garbage.push_back(h);
            return h;
        }
    }

    TCanvas* reg_canvas(const char* name, const char* title, int width, int height)
    {
        if (box)
            return box->reg_canvas(name, title, width, height);
        else
        {
            auto c = new TCanvas(name, title, width, height);
            garbage.push_back(c);
            return c;
        }
    }

protected:
    pandora::pandora* box{nullptr};

private:
    basic_context ctx; //||
    std::vector<TObject*> garbage;

    // #ifdef HAVE_HISTASYMMERRORS
    // 	TH2DA * hSignalCounter;
    // #else
    // 	TH2D * hSignalCounter;			//->	// discrete X-Y, signal extracted
    // #endif
    std::unique_ptr<TH1> signal_histogram;  //->	// discrete X-Y, signal extracted
    std::unique_ptr<TCanvas> signal_canvas; //->

protected:
    TString drawOpts;

    // ClassDef(DistributionFactory, 1);
};

/// Fit callback
using FitCallback = std::function<void(distribution* fac, basic_distribution* sigfac, int fit_res, TH1* h, int x_pos,
                                       int y_pos, int z_pos)>;

class MIDAS_EXPORT distribution : public basic_distribution
{
protected:
    // distribution();

public:
    distribution(const context& ctx, pandora::pandora* pbox = nullptr);
    distribution(const distribution&) = delete;
    distribution(distribution&&) = delete;
    virtual ~distribution();

    auto operator==(const distribution& fa) -> bool;

    auto operator=(const distribution& fa) -> distribution& = delete;
    auto operator=(distribution&& fa) -> distribution& = delete;

    virtual auto fill(Float_t weight = 1.0) -> void override;

    virtual auto prepare() -> void override;
    virtual auto reinit() -> void override;
    virtual auto finalize(const char* draw_opts = nullptr) -> void override;
    // virtual auto reset() -> void override; FIXME

    virtual auto print() const -> void override;
    virtual auto validate() const -> bool override;

    auto transform_d(std::function<void(TH1* h)> transform_function) -> void
    {
        basic_distribution::transform(transform_function);
    }
    auto transform_v(std::function<void(TH1* h)> transform_function) -> void;
    auto transform(std::function<void(TH1* h)> transform_function) -> void
    {
        transform_d(transform_function);
        transform_v(transform_function);
    }

    auto transform_d(std::function<void(TCanvas* h)> transform_function) -> void
    {
        basic_distribution::transform(transform_function);
    }
    auto transform_v(std::function<void(TCanvas* h)> transform_function) -> void;
    auto transform(std::function<void(TCanvas* h)> transform_function) -> void
    {
        transform_d(transform_function);
        transform_v(transform_function);
    }

    virtual void applyBinomErrors(TH1* N) override;

    auto save(TFile* f /* = nullptr*/, bool verbose = false) -> bool override;
    auto save(const char* filename /* = nullptr*/, bool verbose = false) -> bool override;

    void fit_cells_hists(basic_distribution* sigfac, hf::fitter& hf, hf::fit_entry& stdfit, bool integral_only = false);
    bool fit_cell_hist(TH1* hist, hf::fit_entry* hfp, double min_entries = 0);

    void set_fit_callback(FitCallback cb) { fit_callback = std::move(cb); }
    virtual void prepare_cells_canvas();

private:
    context ctx;
    std::unique_ptr<observable> cells;
    TObjArray* objects_fits; //!
    FitCallback fit_callback;

    // ClassDef(DifferentialFactory, 1);
};

template <typename Ctx> void load_from_file() {}

template <class T> auto scale(T* fac, Float_t factor) -> void
{
    fac->transform([&](TH1* h) { h->Scale(factor); });
}

template <class T> auto bin_normalization(T* fac) -> void
{
    fac->transform([](TH1* h) { h->Scale(1.0 / (h->GetXaxis()->GetBinWidth(1) * h->GetYaxis()->GetBinWidth(1))); });
}

/// Apply angula distribution to a histograms. Makes sens only if the x-axis is a cosTheta in cm frame of the system in
/// which the distribution exists.
/// @param fac the midas::distribution object
/// @param a2,a4 the parameters
template <class T>
auto apply_ang_distribution(T* fac, double a2, double a4, double corr_a2 = 0.0, double corr_a4 = 0.0) -> void
{
    fac->transform(
        [&](TH1* h)
        {
            TF1 f("local_legpol", "angdist", -1, 1);
            f.SetParameter(0, 1.0);
            f.SetParameter(1, a2);
            f.SetParameter(2, a4);

            bool has_corr = false;
            std::unique_ptr<TF1> f_corr;
            if (corr_a2 != 0.0 or corr_a4 != 0.0)
            {
                has_corr = true;
                f_corr = std::unique_ptr<TF1>(new TF1("local_legpol_corr", "angdist", -1, 1));

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

                auto angmap = f.Integral(bin_l, bin_r);

                if (has_corr) { corr_factor = f_corr->Integral(bin_l, bin_r); }
                else { angmap /= bin_w; }

                auto scaling_factor = angmap / corr_factor;
                for (auto y = 1; y <= bins_y; ++y)
                {
                    auto tmp_val = h->GetBinContent(x, y);
                    h->SetBinContent(x, y, tmp_val * scaling_factor);
                }
            }
        });
}

}; // namespace midas

#endif // MIDAS_HPP

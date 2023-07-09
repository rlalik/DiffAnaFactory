#ifndef MIDAS_DETAILS_H
#define MIDAS_DETAILS_H

#include "midas.hpp"

#include <TString.h>

namespace midas
{

namespace
{
/// Based on
/// https://stackoverflow.com/questions/27490762/how-can-i-convert-to-size-t-from-int-safely
constexpr auto size_t2int(size_t val) -> int
{
    return (val <= std::numeric_limits<int>::max()) ? static_cast<int>(val) : -1;
}
constexpr auto int2size_t(int val) -> size_t { return (val < 0) ? __SIZE_MAX__ : static_cast<size_t>(val); }

constexpr auto f2d(float x) -> double { return static_cast<double>(x); }

} // namespace

class MIDAS_EXPORT observable : public TObject
{
public:
    observable(dimension dist_dim, const std::string& name, TH1* hist, const axis_config& v_axis,
               const std::string& dir_and_name, pandora::pandora* sf);
    virtual ~observable();

    auto get_bins_x() const -> Int_t { return nbins_x; }
    auto get_bins_y() const -> Int_t { return nbins_y; }
    auto get_bins_z() const -> Int_t { return nbins_z; }

    auto get_bin(Int_t x, Int_t y = 0, Int_t z = 0) const -> Int_t;
    auto reverse_bin(Int_t bin, Int_t& x) const -> bool;
    auto reverse_bin(Int_t bin, Int_t& x, Int_t& y) const -> bool;
    auto reverse_bin(Int_t bin, Int_t& x, Int_t& y, Int_t& z) const -> bool;

    TH1D* get_hist(Int_t x, Int_t y = 0, Int_t z = 0);
    TH1D* find_hist(Double_t x, Double_t y = 0.0, Double_t z = 0.0);
    TCanvas* get_canvas(Int_t x, Int_t y = 0);
    TVirtualPad* get_pad(Int_t x, Int_t y = 0, Int_t z = 0);

    auto get_hists_number() const -> Int_t { return nhists; }
    auto get_hist_by_index(int n) -> TH1D* { return histograms[int2size_t(n)]; }
    auto get_hist_by_index(int n) const -> const TH1D* { return histograms[int2size_t(n)]; }

    auto get_canvas_number() const -> Int_t { return ncanvases; }
    auto get_canvas_by_index(int n) -> TCanvas* { return canvases[int2size_t(n)]; }
    auto get_canvas_by_index(int n) const -> const TCanvas* { return canvases[int2size_t(n)]; }

    auto fill_1d(Double_t x, Double_t v, Double_t w = 1.0) -> void;
    auto fill_2d(Double_t x, Double_t y, Double_t v, Double_t w = 1.0) -> void;
    auto fill_3d(Double_t x, Double_t y, Double_t z, Double_t v, Double_t w = 1.0) -> void;

    auto print() const -> void;
    // ExtraDimensionMapper & operator=(const ExtraDimensionMapper & fa);

private:
    auto map_1d(const axis_config& v_axis) -> void;
    auto map_2d(const axis_config& v_axis) -> void;
    auto map_3d(const axis_config& v_axis) -> void;
    auto format_name(Int_t x, Int_t y = 0, Int_t z = 0) -> TString;
    auto format_canvas_name(Int_t x, Int_t y = 0) -> TString;

    template <typename T, typename... Types>
    T* reg_hist(const char* name, const char* title, Types... arguments) {
        if (box)
            return box->reg_hist<T>(name, title, arguments...);
        else {
            auto h = new T(name, title, arguments...);
            garbage.push_back(h);
            return h;
        }
    }

    TCanvas* reg_canvas(const char* name, const char* title, int width, int height) {
        if (box)
            return box->reg_canvas(name, title, width, height);
        else {
            auto c = new TCanvas(name, title, width, height);
            garbage.push_back(c);
            return c;
        }
    }

private:
    pandora::pandora* box;
    std::vector<TObject*> garbage;
    dimension dim;
    axis_config axis; //||
    std::string prefix_name;

    Int_t nhists;
    Int_t nbins_x, nbins_y, nbins_z;
    Int_t ncanvases;

    TH1* ref_hist;
    std::vector<TH1D*> histograms;  //!
    std::vector<TCanvas*> canvases; //!

    // ClassDef(ExtraDimensionMapper, 1);
};

namespace detail
{

auto dim_to_int(dimension dim) -> int;

auto copyHistogram(TH1* src, TH1* dst, bool with_functions = true) -> bool;

struct json_file_info
{
    bool found;
    TString path;
};

auto find_json_file(const char* initial_path, const char* filename, int search_depth = -1) -> json_file_info;

// auto json_read_TString_key(const Json::Value& jsondata, const char* key, TString& target) -> bool;
// auto json_read_int_key(const Json::Value& jsondata, const char* key, int& target) -> bool;
// auto json_read_uint_key(const Json::Value& jsondata, const char* key, uint& target) -> bool;
// auto json_read_float_key(const Json::Value& jsondata, const char* key, float& target) -> bool;
// auto json_read_double_key(const Json::Value& jsondata, const char* key, double& target) -> bool;

} // namespace detail

} // namespace midas

#endif /* MIDAS_DETAILS_H*/

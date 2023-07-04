#ifndef MIDAS_DETAILS_H
#define MIDAS_DETAILS_H

#include "midas.hpp"

#include <TString.h>

#include <json/json.h>

namespace midas
{

class MIDAS_EXPORT observable : public TObject
{
public:
    observable(dimension dist_dim, const std::string& name, TH1* hist, const axis_config& v_axis,
                         const std::string& dir_and_name, RT::Pandora* sf);
    virtual ~observable();

    auto getBinsX() const -> Int_t { return nbins_x; }
    auto getBinsY() const -> Int_t { return nbins_y; }
    auto getBinsZ() const -> Int_t { return nbins_z; }

    auto getBin(Int_t x, Int_t y = 0, Int_t z = 0) const -> Int_t;
    auto reverseBin(Int_t bin, Int_t& x) const -> bool;
    auto reverseBin(Int_t bin, Int_t& x, Int_t& y) const -> bool;
    auto reverseBin(Int_t bin, Int_t& x, Int_t& y, Int_t& z) const -> bool;

    TH1D* get_hist(Int_t x, Int_t y = 0, Int_t z = 0);
    TH1D* find_hist(Double_t x, Double_t y = 0.0, Double_t z = 0.0);
    TCanvas* get_canvas(Int_t x, Int_t y = 0);
    TVirtualPad* get_pad(Int_t x, Int_t y = 0, Int_t z = 0);

    auto get_hists_number() const -> Int_t { return nhists; }
    auto get_hist_by_index(int n) -> TH1D* { return histograms[n]; }
    auto get_hist_by_index(int n) const -> const TH1D* { return histograms[n]; }

    auto get_canvas_number() const -> Int_t { return ncanvases; }
    auto get_canvas_by_index(int n) -> TCanvas* { return canvases[n]; }
    auto get_canvas_by_index(int n) const -> const TCanvas* { return canvases[n]; }

    auto Fill1D(Float_t x, Float_t v, Float_t w = 1.0) -> void;
    auto Fill2D(Float_t x, Float_t y, Float_t v, Float_t w = 1.0) -> void;
    auto Fill3D(Float_t x, Float_t y, Float_t z, Float_t v, Float_t w = 1.0) -> void;

    auto print() const -> void;
    // ExtraDimensionMapper & operator=(const ExtraDimensionMapper & fa);
private:
    auto map1D(const axis_config& v_axis) -> void;
    auto map2D(const axis_config& v_axis) -> void;
    auto map3D(const axis_config& v_axis) -> void;
    auto formatName(Int_t x, Int_t y = 0, Int_t z = 0) -> TString;
    auto formatCanvasName(Int_t x, Int_t y = 0) -> TString;

public:
    RT::Pandora * box;
    dimension dim;
    axis_config axis; //||
    std::string prefix_name;

    Int_t nhists;
    Int_t nbins_x, nbins_y, nbins_z;
    Int_t ncanvases;

    TH1* ref_hist;
    TH1D** histograms;  //!
    TCanvas** canvases; //!

    // 	TObjArray * objectsFits;

    // ClassDef(ExtraDimensionMapper, 1);

private:
    // 	FitCallback * fitCallback;
};

namespace detail
{

auto dim_to_int(dimension dim) -> int;

auto copyHistogram(TH1* src, TH1* dst, bool with_functions = true) -> bool;

struct json_file_info {
    bool found;
    TString path;
};

auto find_json_file(const char* initial_path, const char* filename, int search_depth = -1) -> json_file_info;

auto json_read_TString_key(const Json::Value& jsondata, const char* key, TString& target) -> bool;
auto json_read_int_key(const Json::Value& jsondata, const char* key, int& target) -> bool;
auto json_read_uint_key(const Json::Value& jsondata, const char* key, uint& target) -> bool;
auto json_read_float_key(const Json::Value& jsondata, const char* key, float& target) -> bool;
auto json_read_double_key(const Json::Value& jsondata, const char* key, double& target) -> bool;

} // namespace detail

} // namespace midas

#endif /* MIDAS_DETAILS_H*/

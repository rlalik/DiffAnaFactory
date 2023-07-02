#ifndef MIDAS_DETAILS_H
#define MIDAS_DETAILS_H

#include "midas.hpp"

#include <TString.h>

#include <json/json.h>

namespace midas
{

class MIDAS_EXPORT observable : public TObject, public RT::Pandora
{
public:
    observable(dimension dist_dim, const std::string& name, TH1* hist, const axis_config& v_axis,
                         const std::string& dir_and_name);
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

    TH1D* get(Int_t x, Int_t y = 0, Int_t z = 0);
    TH1D* find(Double_t x, Double_t y = 0.0, Double_t z = 0.0);
    TCanvas* getCanvas(Int_t x, Int_t y = 0);
    TVirtualPad* getPad(Int_t x, Int_t y = 0, Int_t z = 0);

    auto getNHists() const -> Int_t { return nhists; }
    auto operator[](int n) -> TH1D* { return histograms[n]; }
    auto operator[](int n) const -> const TH1D* { return histograms[n]; }

    auto Fill1D(Float_t x, Float_t v, Float_t w = 1.0) -> void;
    auto Fill2D(Float_t x, Float_t y, Float_t v, Float_t w = 1.0) -> void;
    auto Fill3D(Float_t x, Float_t y, Float_t z, Float_t v, Float_t w = 1.0) -> void;

    // ExtraDimensionMapper & operator=(const ExtraDimensionMapper & fa);
private:
    auto map1D(const axis_config& v_axis) -> void;
    auto map2D(const axis_config& v_axis) -> void;
    auto map3D(const axis_config& v_axis) -> void;
    auto formatName(Int_t x, Int_t y = 0, Int_t z = 0) -> TString;
    auto formatCanvasName(Int_t x, Int_t y = 0) -> TString;

public:
    dimension dim;
    axis_config axis; //||
    std::string prefix_name;

    Int_t nhists;
    Int_t nbins_x, nbins_y, nbins_z;

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

constexpr auto dim_to_int(dimension dim) -> int
{
    switch (dim)
    {
        case dimension::NODIM:
            return 0;
        case dimension::DIM1:
            return 1;
        case dimension::DIM2:
            return 2;
        case dimension::DIM3:
            return 3;
        default:
            return -1;
    }
}

auto copyHistogram(TH1* src, TH1* dst, bool with_functions = true) -> bool;

auto jsonReadTStringKey(const Json::Value& jsondata, const char* key, TString& target) -> bool;
auto jsonReadIntKey(const Json::Value& jsondata, const char* key, int& target) -> bool;
auto jsonReadUIntKey(const Json::Value& jsondata, const char* key, uint& target) -> bool;
auto jsonReadFloatKey(const Json::Value& jsondata, const char* key, float& target) -> bool;
auto jsonReadDoubleKey(const Json::Value& jsondata, const char* key, double& target) -> bool;

} // namespace detail

} // namespace midas

#endif /* MIDAS_DETAILS_H*/

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

#include "detail.hpp"

#include "midas.hpp"

#include <pandora.hpp>

#include <fmt/core.h>

#include <TCanvas.h>
#include <TH1.h>
#include <TSystem.h>
#include <TVirtualArray.h>

// const Option_t h1opts[] = "h,E1";

// const TString flags_fit_a = "B,Q,0";
// const TString flags_fit_b = "";

namespace midas
{

observable::observable(dimension dist_dim, const std::string& ctx_name, TH1* hist, const axis_config& v_axis,
                       const std::string& dir_and_name, pandora::pandora* sf)
    : box(sf), dim(dist_dim), axis(v_axis), prefix_name(dir_and_name), ref_hist(hist)
{
    nbins_x = hist->GetNbinsX();
    nbins_y = hist->GetNbinsY();
    nbins_z = hist->GetNbinsZ();
    nhists = nbins_x * nbins_y * nbins_z;

    histograms.resize(int2size_t(nhists));
    if (dimension::DIM1 == dim)
    {
        ncanvases = 1;
        canvases.resize(int2size_t(ncanvases));
        map_1d(axis);
    }
    else if (dimension::DIM2 == dim)
    {
        ncanvases = nbins_x;
        canvases.resize(int2size_t(ncanvases));
        map_2d(axis);
    }
    else if (dimension::DIM3 == dim)
    {
        ncanvases = nbins_x * nbins_y;
        canvases.resize(int2size_t(ncanvases));
        map_3d(axis);
    }
}

observable::~observable()
{
    gSystem->ProcessEvents();

    for (auto& o : garbage)
        delete o;
}

auto observable::get_bin(Int_t x, Int_t y, Int_t z) const -> Int_t { return z * (nbins_x * nbins_y) + y * nbins_x + x; }

auto observable::reverse_bin(Int_t bin, Int_t& x) const -> bool
{
    if (dim != dimension::DIM1) return false;
    if (bin < 0 or bin >= nhists) return false;
    x = bin % nbins_x;
    return true;
}

auto observable::reverse_bin(Int_t bin, Int_t& x, Int_t& y) const -> bool
{
    if (dim < dimension::DIM2)
    {
        y = 0;
        return reverse_bin(bin, x);
    }
    if (dim != dimension::DIM2) return false;
    if (bin < 0 or bin >= nhists) return false;
    x = bin % nbins_x;
    y = bin / nbins_x;
    return true;
}

auto observable::reverse_bin(Int_t bin, Int_t& x, Int_t& y, Int_t& z) const -> bool
{
    if (dim < dimension::DIM3)
    {
        z = 0;
        return reverse_bin(bin, x, y);
    }
    if (dim != dimension::DIM3) return false;
    if (bin < 0 or bin >= nhists) return false;
    x = bin % nbins_x;
    y = (bin / nbins_x) % nbins_y;
    z = bin / (nbins_x * nbins_y);
    return true;
}

auto observable::map_1d(const axis_config& v_axis) -> void
{
    for (Int_t i = 0; i < nbins_x; ++i)
    {
        auto fname = format_name(i);
        if (v_axis.get_bins_array())
        {
            histograms[int2size_t(get_bin(i))] = reg_hist<TH1D>(fname.Data(), v_axis.format_hist_string(fname.Data()),
                                                                v_axis.get_bins(), v_axis.get_bins_array());
        }
        else
        {
            histograms[int2size_t(get_bin(i))] = reg_hist<TH1D>(fname.Data(), v_axis.format_hist_string(fname.Data()),
                                                                v_axis.get_bins(), v_axis.get_min(), v_axis.get_max());
        }
    }

    auto fname = format_canvas_name(0);
    canvases[0] = reg_canvas(fname.Data(), fname.Data(), 800, 600);
    canvases[0]->DivideSquare(nbins_x);
}

auto observable::map_2d(const axis_config& v_axis) -> void
{
    for (Int_t i = 0; i < nbins_x; ++i)
    {
        for (Int_t j = 0; j < nbins_y; ++j)
        {
            auto fname = format_name(i, j);
            if (v_axis.get_bins_array())
            {
                histograms[int2size_t(get_bin(i, j))] = reg_hist<TH1D>(
                    fname.Data(), v_axis.format_hist_string(fname.Data()), v_axis.get_bins(), v_axis.get_bins_array());
            }
            else
            {
                histograms[int2size_t(get_bin(i, j))] =
                    reg_hist<TH1D>(fname.Data(), v_axis.format_hist_string(fname.Data()), v_axis.get_bins(),
                                   v_axis.get_min(), v_axis.get_max());
            }
        }
        auto fname = format_canvas_name(i);
        canvases[int2size_t(i)] = reg_canvas(fname.Data(), fname.Data(), 800, 600);
        canvases[int2size_t(i)]->DivideSquare(nbins_y);
    }
}

auto observable::map_3d(const axis_config& v_axis) -> void
{
    for (Int_t i = 0; i < nbins_x; ++i)
    {
        for (Int_t j = 0; j < nbins_y; ++j)
        {
            for (Int_t k = 0; k < nbins_z; ++k)
            {
                auto fname = format_name(i, j, k);
                if (v_axis.get_bins_array())
                {
                    histograms[int2size_t(get_bin(i, j, k))] =
                        reg_hist<TH1D>(fname.Data(), v_axis.format_hist_string(fname.Data()), v_axis.get_bins(),
                                       v_axis.get_bins_array());
                }
                else
                {
                    histograms[int2size_t(get_bin(i, j, k))] =
                        reg_hist<TH1D>(fname.Data(), v_axis.format_hist_string(fname.Data()), v_axis.get_bins(),
                                       v_axis.get_min(), v_axis.get_max());
                }
            }
            auto fname = format_canvas_name(i, j);
            canvases[int2size_t(i + j * nbins_x)] = reg_canvas(fname.Data(), fname.Data(), 800, 600);
            canvases[int2size_t(i + j * nbins_x)]->DivideSquare(nbins_z);
        }
    }
}

auto observable::format_name(Int_t x, Int_t y, Int_t z) -> TString
{
    char name[200];
    sprintf(name, prefix_name.c_str(), 'h');

    if (dimension::DIM1 == dim)
        return TString::Format("%s_X%02d", name, x);
    else if (dimension::DIM2 == dim)
        return TString::Format("%s_X%02d_Y%02d", name, x, y);
    else if (dimension::DIM3 == dim)
        return TString::Format("%s_X%02d_Y%02d_Z%02d", name, x, y, z);
    else
        return "";
}

auto observable::format_canvas_name(Int_t x, Int_t y) -> TString
{
    char name[200];
    sprintf(name, prefix_name.c_str(), 'c');

    if (dimension::DIM1 == dim)
        return TString::Format("%s_X", name);
    else if (dimension::DIM2 == dim)
        return TString::Format("%s_X%02d", name, x);
    else if (dimension::DIM3 == dim)
        return TString::Format("%s_X%02d_Y%02d", name, x, y);
    else
        return "";
}

// observable & observable::operator=(const observable & edm)
// {
// 	observable * nthis = this;//new observable(fa.ctx);
//
// 	nthis->axis = edm.axis;
// 	nthis->nhists = edm.nhists;
//   objectsFits = new TObjArray();
// nthis->objectsFits->SetName(ctx.histPrefix + "Fits");
// 	for (uint i = 0; i < nthis->nhists; ++i)
// 	{
// 		copyHistogram(edm.histograms[i], nthis->histograms[i]);
// 	}
//
// 	return *nthis;
// }

auto observable::get_hist(Int_t x, Int_t y, Int_t z) -> TH1D*
{
    if (x >= nbins_x or y >= nbins_y or z >= nbins_z) return nullptr;

    return histograms[int2size_t(get_bin(x, y, z))];
}

auto observable::get_canvas(Int_t x, Int_t y) -> TCanvas*
{
    if (dimension::DIM3 == dim)
    {
        if (x >= nbins_x or y >= nbins_y) return nullptr;

        return canvases[int2size_t(x + y * nbins_x)];
    }
    if (dimension::DIM2 == dim)
    {
        if (x >= nbins_x) return nullptr;

        return canvases[int2size_t(x)];
    }

    return canvases[0];
}

auto observable::get_pad(Int_t x, Int_t y, Int_t z) -> TVirtualPad*
{
    TCanvas* can = get_canvas(x, y);

    if (dim == dimension::DIM1) return can->GetPad(1 + x);
    if (dim == dimension::DIM2) return can->GetPad(1 + y);
    if (dim == dimension::DIM3) return can->GetPad(1 + z);

    return can->GetPad(0);
}

TH1D* observable::find_hist(Double_t x, Double_t y, Double_t z)
{
    auto bin = ref_hist->FindBin(x, y, z);
    Int_t bx, by, bz;
    ref_hist->GetBinXYZ(bin, bx, by, bz);
    return histograms[int2size_t(get_bin(bx - 1, by - 1, bz - 1))];
}

auto observable::fill_1d(Double_t x, Double_t v, Double_t w) -> void
{
    auto bin = ref_hist->FindBin(x);
    Int_t bx, by, bz;
    ref_hist->GetBinXYZ(bin, bx, by, bz);
    if (bx > 0 && bx <= nbins_x) histograms[int2size_t(get_bin(bx - 1))]->Fill(v, w);
}

auto observable::fill_2d(Double_t x, Double_t y, Double_t v, Double_t w) -> void
{
    auto bin = ref_hist->FindBin(x, y);
    Int_t bx, by, bz;
    ref_hist->GetBinXYZ(bin, bx, by, bz);
    if (bx > 0 && bx <= nbins_x && by > 0 && by <= nbins_y) histograms[int2size_t(get_bin(bx - 1, by - 1))]->Fill(v, w);
}

auto observable::fill_3d(Double_t x, Double_t y, Double_t z, Double_t v, Double_t w) -> void
{
    auto bin = ref_hist->FindBin(x, y, z);
    Int_t bx, by, bz;
    ref_hist->GetBinXYZ(bin, bx, by, bz);
    if (bx > 0 && bx <= nbins_x && by > 0 && by <= nbins_y && bz > 0 && bz <= nbins_z)
        histograms[int2size_t(get_bin(bx - 1, by - 1, bz - 1))]->Fill(v, w);
}

auto observable::print() const -> void
{
    fmt::print(" Observable: bins x={}  y={}  z={}\n", nbins_x, nbins_y, nbins_z);
}

} // namespace midas

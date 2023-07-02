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

#include "detail.hpp"

#include <fmt/core.h>

#include <fstream>
#include <iostream>

namespace midas
{

context::context()
{
    // basic
    // x and y binning for full range

    // cut range when useCut==kTRUE
    // 	cutMin = cutMax = 0;

    // variables to use for diff analysis
    var_weight = 0;
    // variable used for cuts when cutCut==kTRUE
}

context::context(TString context_name, dimension context_dim, axis_config v_axis)
    : basic_context(context_name, context_dim), v(std::move(v_axis))
{
}

context::context(TString context_name, axis_config x_axis, axis_config v_axis)
    : basic_context(context_name, x_axis), v(std::move(v_axis))
{
}

context::context(TString context_name, axis_config x_axis, axis_config y_axis, axis_config v_axis)
    : basic_context(context_name, x_axis, y_axis), v(std::move(v_axis))
{
}

context::context(TString context_name, axis_config x_axis, axis_config y_axis, axis_config z_axis, axis_config v_axis)
    : basic_context(context_name, x_axis, y_axis, z_axis), v(std::move(v_axis))
{
}

context::context(const basic_context& ctx) : basic_context(ctx) {}

context::context(const context& ctx) : basic_context(ctx)
{
    *this = ctx;
    name = ctx.name;
}

context::~context() {}

bool context::configureFromJson(const char* ctx_name)
{
    std::ifstream ifs(json_fn.Data());
    if (!ifs.is_open()) return false;

    fmt::print("  Found JSON config file for {}\n", ctx_name);
    Json::Value ana, cfg, axis;
    Json::Reader reader;

    bool parsing_success = reader.parse(ifs, ana);

    if (!parsing_success)
    {
        fmt::print("  Parsing failed\n");
        return false;
    }
    else
        fmt::print("  Parsing successfull\n");

    if (!ana.isMember(ctx_name))
    {
        fmt::print("  No data for {}\n", ctx_name);
        return false;
    }

    cfg = ana[ctx_name];

    constexpr size_t axis_num = 4;
    const char* axis_labels[axis_num] = {"x", "y", "z", "v"};
    axis_config* axis_ptrs[axis_num] = {&x, &y, &z, &v};

    for (size_t i = 0; i < axis_num; ++i)
    {
        if (!cfg.isMember(axis_labels[i])) continue;

        axis = cfg[axis_labels[i]];

        // 		jsonReadIntKey(axis, "bins", axis_ptrs[i]->bins);
        Int_t bins;
        detail::jsonReadIntKey(axis, "bins", bins);
        Float_t min, max;
        detail::jsonReadFloatKey(axis, "min", min);
        detail::jsonReadFloatKey(axis, "max", max);
        // 		jsonReadTStringKey(axis, "title", axis_ptrs[i]->title);
        TString label, unit;
        detail::jsonReadTStringKey(axis, "label", label);
        detail::jsonReadTStringKey(axis, "unit", unit);

        axis_ptrs[i]->set_bins(bins, min, max).set_label(label).set_unit(unit);
    }

    ifs.close();
    return true;
}

bool context::configureToJson(const char* context_name, const char* jsonfile)
{
    (void)jsonfile;

    Json::Value ana, cfg, axis;

    cfg["title"] = "d^{2}N/dp_{t}dy.{cm}";

    axis["bins"] = 100;
    axis["min"] = 0;
    axis["max"] = 100;
    axis["label"] = "xlabel";
    axis["var"] = "y.{cm}";

    cfg["x"] = axis;

    axis["bins"] = 100;
    axis["min"] = 0;
    axis["max"] = 100;
    axis["label"] = "ylabel";
    axis["var"] = "p_{t}";

    cfg["y"] = axis;

    axis["bins"] = 100;
    axis["min"] = 0;
    axis["max"] = 100;
    axis["label"] = "Vlabel";
    axis["var"] = "none";

    cfg["V"] = axis;

    ana[context_name] = cfg;

    std::cout << ana;

    // 	Json::StyledWriter sw;
    // 	std::cout << sw.write(ana);

    // 	Json::FastWriter fw;
    // 	std::cout << fw.write(ana);

    return true;
}

context& context::operator=(const basic_context& ctx)
{
    if (this == &ctx) return *this;
    // 	histPrefix = ctx.histPrefix;
    *this = ctx;

    return *this;
}

context& context::operator=(const context& ctx)
{
    if (this == &ctx) return *this;
    // 	histPrefix = ctx.histPrefix;
    basic_context::operator=(ctx);
    name = ctx.name;
    v = ctx.v;

    return *this;
}

bool context::operator==(const context& ctx)
{
    bool res = static_cast<basic_context>(*this) == static_cast<basic_context>(ctx);
    if (!res) return false;

    if (this->v != ctx.v)
    {
        // fprintf(stderr, "Different number of z bins: %d vs %d\n", this->V.bins, ctx.V.bins); FIXME
        return false;
    }

    return true;
}

bool context::operator!=(const context& ctx) { return !operator==(ctx); }

void context::print() const
{
    basic_context::print();
    v.print();
    printf(" label: %s  unit: %s\n", context_label.Data(), context_unit.Data());
}

}; // namespace midas

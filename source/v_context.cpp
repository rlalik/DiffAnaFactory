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

#include <json/json.h>

#include <fstream>
#include <iostream>

#define PR(x) std::cout << "++DEBUG: " << #x << " = |" << x << "| (" << __FILE__ << ", " << __LINE__ << ")\n";

namespace midas
{

v_context::v_context()
{
    // basic
    // x and y binning for full range

    // cut range when useCut==kTRUE
    // 	cutMin = cutMax = 0;

    // variables to use for diff analysis
    var_weight = 0;
    // variable used for cuts when cutCut==kTRUE
}

v_context::v_context(const v_context& ctx) : context(ctx)
{
    *this = ctx;
    name = ctx.name;
}

v_context::~v_context() {}

extern bool jsonReadTStringKey(const Json::Value& jsondata, const char* key, TString& target);
extern bool jsonReadIntKey(const Json::Value& jsondata, const char* key, int& target);
extern bool jsonReadUIntKey(const Json::Value& jsondata, const char* key, uint& target);
extern bool jsonReadFloatKey(const Json::Value& jsondata, const char* key, float& target);
extern bool jsonReadDoubleKey(const Json::Value& jsondata, const char* key, double& target);

bool v_context::configureFromJson(const char* name)
{
    std::ifstream ifs(json_fn.Data());
    if (!ifs.is_open()) return false;

    std::cout << "  Found JSON config file for " << name << std::endl;
    Json::Value ana, cfg, axis;
    Json::Reader reader;

    bool parsing_success = reader.parse(ifs, ana);

    if (!parsing_success)
    {
        std::cout << "  Parsing failed\n";
        return false;
    }
    else
        std::cout << "  Parsing successfull\n";

    if (!ana.isMember(name))
    {
        std::cout << "  No data for " << name << std::endl;
        return false;
    }

    cfg = ana[name];

    const size_t axis_num = 3;
    const char* axis_labels[axis_num] = {"x", "y", "V"};
    axis_config* axis_ptrs[axis_num] = {&x, &y, &V};

    for (uint i = 0; i < axis_num; ++i)
    {
        if (!cfg.isMember(axis_labels[i])) continue;

        axis = cfg[axis_labels[i]];

        // 		jsonReadIntKey(axis, "bins", axis_ptrs[i]->bins);
        UInt_t bins;
        jsonReadUIntKey(axis, "bins", bins);
        Float_t min, max;
        jsonReadFloatKey(axis, "min", min);
        jsonReadFloatKey(axis, "max", max);
        // 		jsonReadTStringKey(axis, "title", axis_ptrs[i]->title);
        TString label, unit;
        jsonReadTStringKey(axis, "label", label);
        jsonReadTStringKey(axis, "unit", unit);

        axis_ptrs[i]->set_bins(bins, min, max).set_label(label).set_unit(unit);
    }

    ifs.close();
    return true;
}

bool v_context::configureToJson(const char* name, const char* jsonfile)
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

    ana[name] = cfg;

    std::cout << ana;

    // 	Json::StyledWriter sw;
    // 	std::cout << sw.write(ana);

    // 	Json::FastWriter fw;
    // 	std::cout << fw.write(ana);

    return true;
}

v_context& v_context::operator=(const v_context& ctx)
{
    if (this == &ctx) return *this;
    // 	histPrefix = ctx.histPrefix;
    context::operator=(ctx);
    name = ctx.name;
    V = ctx.V;

    return *this;
}

bool v_context::operator==(const v_context& ctx)
{
    bool res = (context) * this == (context)ctx;
    if (!res) return false;

    if (this->V != ctx.V)
    {
        // fprintf(stderr, "Different number of z bins: %d vs %d\n", this->V.bins, ctx.V.bins); FIXME
        return false;
    }

    return true;
}

bool v_context::operator!=(const v_context& ctx) { return !operator==(ctx); }

void v_context::print() const
{
    context::print();
    V.print();
    printf(" label: %s  unit: %s\n", label.Data(), unit.Data());
}

}; // namespace midas

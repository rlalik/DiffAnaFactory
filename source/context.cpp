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
#include <sys/stat.h>

#define PR(x) std::cout << "++DEBUG: " << #x << " = |" << x << "| (" << __FILE__ << ", " << __LINE__ << ")\n";

namespace midas
{

context::context() : TNamed(), dim(NOINIT), var_weight(nullptr), json_found(false) {}

context::context(dimension dim) : TNamed(), dim(dim), var_weight(nullptr), json_found(false) {}

context::context(const context& ctx) : TNamed()
{
    *this = ctx;
    name = ctx.name;
}

bool context::update()
{
    if (0 == hist_name.Length()) hist_name = name;

    if (0 == dir_name.Length()) dir_name = hist_name;

    if (name.EndsWith("Ctx"))
        SetName(name);
    else
        SetName(name + "Ctx");

    format_diff_axis();

    return true;
}

int context::validate() const
{
    if (0 == name.Length()) return 1;
    if (DIM0 <= dim && x.get_bins() and !x.get_var()) return 11;
    if (DIM1 <= dim && x.get_bins() and !x.get_var()) return 11;
    if (DIM2 <= dim && y.get_bins() and !y.get_var()) return 12;
    if (DIM3 == dim && z.get_bins() and !z.get_var()) return 13;

    return 0;
    // 	return update();
}

context::~context() {}

bool jsonReadTStringKey(const Json::Value& jsondata, const char* key, TString& target)
{
    if (jsondata.isMember(key))
    {
        target = jsondata[key].asCString();
        std::cout << "    + " << key << ": " << target.Data() << std::endl;
        return true;
    }
    return false;
}

bool jsonReadIntKey(const Json::Value& jsondata, const char* key, int& target)
{
    if (jsondata.isMember(key))
    {
        target = jsondata[key].asInt();
        std::cout << "    + " << key << ": " << target << std::endl;
        return true;
    }
    return false;
}

bool jsonReadUIntKey(const Json::Value& jsondata, const char* key, uint& target)
{
    if (jsondata.isMember(key))
    {
        target = jsondata[key].asInt();
        std::cout << "    + " << key << ": " << target << std::endl;
        return true;
    }
    return false;
}

bool jsonReadFloatKey(const Json::Value& jsondata, const char* key, float& target)
{
    if (jsondata.isMember(key))
    {
        target = jsondata[key].asFloat();
        std::cout << "    + " << key << ": " << target << std::endl;
        return true;
    }
    return false;
}

bool jsonReadDoubleKey(const Json::Value& jsondata, const char* key, double& target)
{
    if (jsondata.isMember(key))
    {
        target = jsondata[key].asDouble();
        std::cout << "    + " << key << ": " << target << std::endl;
        return true;
    }
    return false;
}

bool context::configureFromJson(const char* name)
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
    const char* axis_labels[axis_num] = {"x", "y", "z"};
    axis_config* axis_ptrs[axis_num] = {&x, &y, &z};

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

bool context::configureToJson(const char* name, const char* jsonfile)
{
    (void)jsonfile;

    Json::Value ana, cfg, axis;

    cfg["title"] = "d^{2}N/dp_{t}dy.{cm}";

    axis["bins"] = 0;
    axis["min"] = 0;
    axis["max"] = 0;
    axis["label"] = "";
    axis["var"] = "";

    cfg["x"] = axis;
    cfg["y"] = axis;
    cfg["z"] = axis;
    cfg["V"] = axis;

    ana[name] = cfg;

    std::cout << ana;

    // 	Json::StyledWriter sw;
    // 	std::cout << sw.write(ana);

    // 	Json::FastWriter fw;
    // 	std::cout << fw.write(ana);

    return true;
}

bool context::findJsonFile(const char* initial_path, const char* filename, int search_depth)
{
    const size_t max_len = 1024 * 16;
    int depth_counter = 0;
    char* resolv_name = new char[max_len];
    char* test_path = new char[max_len];
    struct stat buffer;

    strncpy(test_path, initial_path, max_len);

    char* ret_val = 0;
    while (true)
    {
        ret_val = realpath(test_path, resolv_name);
        if (!ret_val) break;

        std::string name = resolv_name;
        name += "/";
        name += filename;

        if (stat(name.c_str(), &buffer) == 0)
        {
            json_found = true;
            json_fn = name;
            break;
        }

        strncpy(test_path, resolv_name, max_len);
        strncpy(test_path + strlen(test_path), "/..", 4);

        if (strcmp(resolv_name, "/") == 0) break;

        ++depth_counter;
        if (search_depth >= 0 and (depth_counter > search_depth)) break;
    }

    if (json_found) printf(" Found json config at %s\n", json_fn.Data());

    delete[] resolv_name;
    delete[] test_path;

    return json_found;
}

context& context::operator=(const context& ctx)
{
    if (this == &ctx) return *this;

    dim = ctx.dim;
    // 	ctx_name = ctx.ctx_name;
    dir_name = ctx.dir_name;
    hist_name = ctx.hist_name;
    diff_var_name = ctx.diff_var_name;

    title = ctx.title;
    label = ctx.label;
    unit = ctx.unit;
    axis_text = ctx.axis_text;

    x = ctx.x;
    y = ctx.y;
    z = ctx.z;

    var_weight = ctx.var_weight;

    return *this;
}

bool context::operator==(const context& ctx)
{
    if (this == &ctx) return true;

    if (this->dim != ctx.dim)
    {
        fprintf(stderr, "Not the same dimensions: %d vs %d\n", this->dim, ctx.dim);
        return false;
    }

    if (this->x != ctx.x)
    {
        fprintf(stderr, "Different number of x bins: %d vs %d\n", this->x.get_bins(), ctx.x.get_bins());
        return false;
    }

    if (this->y != ctx.y)
    {
        fprintf(stderr, "Different number of y bins: %d vs %d\n", this->y.get_bins(), ctx.y.get_bins());
        return false;
    }

    if (this->z != ctx.z)
    {
        fprintf(stderr, "Different number of z bins: %d vs %d\n", this->z.get_bins(), ctx.z.get_bins());
        return false;
    }

    return true;
}

bool context::operator!=(const context& ctx) { return !operator==(ctx); }

void context::format_diff_axis()
{
    TString hunit = "1/";
    if (DIM0 <= dim) hunit += x.get_unit().Data();
    if (DIM1 <= dim) hunit += x.get_unit().Data();
    if (DIM2 <= dim) hunit += y.get_unit().Data();
    if (DIM3 == dim) hunit += z.get_unit().Data();

    UInt_t dim_cnt = 0;
    TString htitle;
    if (DIM3 == dim) dim_cnt = 3;
    if (DIM2 == dim) dim_cnt = 2;
    if (DIM1 == dim) dim_cnt = 1;
    if (DIM0 == dim) dim_cnt = 0;

    if (DIM1 < dim)
        htitle = TString::Format("d^{%d}%s/", dim_cnt, diff_var_name.Data());
    else if (DIM1 == dim)
        htitle = TString::Format("d%s/", diff_var_name.Data());
    else
        htitle = TString::Format("%s", diff_var_name.Data());

    if (DIM1 <= dim) htitle += TString("d") + x.get_label().Data();
    if (DIM2 <= dim) htitle += TString("d") + y.get_label().Data();
    if (DIM3 == dim) htitle += TString("d") + z.get_label().Data();

    label = htitle;
    unit = hunit;

    if (unit.Length())
        axis_text = label + " [" + unit + "]";
    else
        axis_text = label;
}

TString context::format_hist_axes(const char* title) const
{
    if (DIM3 == dim)
        return TString::Format("%s;%s%s%s;%s%s%s", title, x.get_label().Data(), x.format_unit().Data(),
                               y.get_label().Data(), y.format_unit().Data(), z.get_label().Data(),
                               z.format_unit().Data());
    else if (DIM2 == dim)
        return TString::Format("%s;%s%s;%s%s", title, x.get_label().Data(), x.format_unit().Data(),
                               y.get_label().Data(), y.format_unit().Data());
    else if (DIM1 == dim)
        return TString::Format("%s;%s%s;Counts [aux]", title, x.get_label().Data(), x.format_unit().Data());

    else if (DIM0 == dim)
        return TString::Format("%s;%s%s;Counts [aux]", title, x.get_label().Data(), x.format_unit().Data());

    return TString::Format("%s;;", title);
}

void context::print() const
{
    printf("Context: %s   Dimensions: %d\n", name.Data(), dim);
    printf(" Name: %s   Hist name: %s   Dir Name: %s\n", name.Data(), hist_name.Data(), dir_name.Data());
    printf(" Var name: %s\n", diff_var_name.Data());
    x.print();
    y.print();
    z.print();
}

}; // namespace midas

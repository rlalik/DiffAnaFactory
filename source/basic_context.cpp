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

#include <json/json.h>

#include <fmt/core.h>

#include <fstream>
#include <sys/stat.h>

namespace midas
{

basic_context::basic_context() : TNamed(), dim(dimension::NODIM), var_weight(nullptr), json_found(false) {}

// context::context(dimension dim) : TNamed(), dim(dim), var_weight(nullptr), json_found(false) {}

basic_context::basic_context(TString context_name, dimension context_dim)
    : TNamed(), dim(context_dim), name(context_name), var_weight(nullptr)
{
}

basic_context::basic_context(TString context_name, axis_config x_axis)
    : TNamed(), dim(dimension::DIM1), name(context_name), x(std::move(x_axis)), var_weight(nullptr)
{
}

basic_context::basic_context(TString context_name, axis_config x_axis, axis_config y_axis)
    : TNamed(), dim(dimension::DIM2), name(context_name), x(std::move(x_axis)), y(std::move(y_axis)),
      var_weight(nullptr)
{
}

basic_context::basic_context(TString context_name, axis_config x_axis, axis_config y_axis, axis_config z_axis)
    : TNamed(), dim(dimension::DIM3), name(context_name), x(std::move(x_axis)), y(std::move(y_axis)),
      z(std::move(z_axis)), var_weight(nullptr)
{
}

auto basic_context::cast(TString new_name, dimension new_dim) const -> basic_context
{
    basic_context new_ctx = *this;
    new_ctx.name = new_name;
    new_ctx.dim = new_dim;
    return new_ctx;
}

auto basic_context::reduce() -> void
{
    if (dim == midas::dimension::DIM1) { throw dimension_error("Cannot reduce single dimension"); }
    else if (dim == midas::dimension::DIM2)
    {
        dim = midas::dimension::DIM1;
        y = axis_config();
    }
    else if (dim == midas::dimension::DIM3)
    {
        dim = midas::dimension::DIM2;
        z = axis_config();
    }
    else { throw dimension_error("Cannot reduce empty dimension"); }
}

auto basic_context::extend() -> axis_config&
{
    if (dim == midas::dimension::DIM3) { throw dimension_error("Cannot create next dimension"); }
    else if (dim == midas::dimension::DIM2)
    {
        dim = midas::dimension::DIM3;
        return z;
    }
    else if (dim == midas::dimension::DIM1)
    {
        dim = midas::dimension::DIM2;
        return y;
    }
    else
    {
        dim = midas::dimension::DIM1;
        return x;
    }
}

auto basic_context::extend(axis_config next_dim) -> void { extend() = std::move(next_dim); }

auto basic_context::get_x() -> axis_config&
{
    if (dim < midas::dimension::DIM1) { throw dimension_error("Dimension 'x' not enabled"); }
    return x;
}

auto basic_context::get_y() -> axis_config&
{
    if (dim < midas::dimension::DIM2) { throw dimension_error("Dimension 'y' not enabled"); }
    return y;
}
auto basic_context::get_z() -> axis_config&
{
    if (dim < midas::dimension::DIM3) { throw dimension_error("Dimension 'z' not enabled"); }
    return z;
}

auto basic_context::expand(axis_config extra_dim) -> context
{
    context vctx = *this;
    vctx.get_v() = std::move(extra_dim);
    return vctx;
}

auto basic_context::update() -> bool
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

int basic_context::validate() const
{
    if (0 == name.Length()) return 1;
    if (dimension::DIM1 <= dim && x.get_bins() and !x.get_var()) return 11;
    if (dimension::DIM2 <= dim && y.get_bins() and !y.get_var()) return 12;
    if (dimension::DIM3 == dim && z.get_bins() and !z.get_var()) return 13;

    return 0;
    // 	return update();
}

bool basic_context::configureFromJson(const char* ctx_name)
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

    const size_t axis_num = 3;
    const char* axis_labels[axis_num] = {"x", "y", "z"};
    axis_config* axis_ptrs[axis_num] = {&x, &y, &z};

    for (uint i = 0; i < axis_num; ++i)
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

bool basic_context::configureToJson(const char* ctx_name, const char* jsonfile)
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

    ana[ctx_name] = cfg;

    std::cout << ana;

    // 	Json::StyledWriter sw;
    // 	std::cout << sw.write(ana);

    // 	Json::FastWriter fw;
    // 	std::cout << fw.write(ana);

    return true;
}

bool basic_context::findJsonFile(const char* initial_path, const char* filename, int search_depth)
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

// context& context::operator=(const context& ctx)
// {
//     if (this == &ctx) return *this;
//
//     // 	ctx_name = ctx.ctx_name;
//     dir_name = ctx.dir_name;
//     hist_name = ctx.hist_name;
//     diff_var_name = ctx.diff_var_name;
//
//     title = ctx.title;
//     label = ctx.label;
//     unit = ctx.unit;
//     axis_text = ctx.axis_text;
//
//     x = ctx.x;
//     y = ctx.y;
//     z = ctx.z;
//
//     var_weight = ctx.var_weight;
//
//     return *this;
// }

bool basic_context::operator==(const basic_context& ctx)
{
    if (this == &ctx) return true;

    if (this->dim != ctx.dim)
    {
        fmt::print(stderr, "Not the same dimensions: {} vs {}\n", detail::dim_to_int(this->dim),
                   detail::dim_to_int(ctx.dim));
        return false;
    }

    if (this->x != ctx.x)
    {
        fmt::print(stderr, "Different number of x bins: {} vs {}\n", this->x.get_bins(), ctx.x.get_bins());
        return false;
    }

    if (this->y != ctx.y)
    {
        fmt::print(stderr, "Different number of y bins: {} vs {}\n", this->y.get_bins(), ctx.y.get_bins());
        return false;
    }

    if (this->z != ctx.z)
    {
        fmt::print(stderr, "Different number of z bins: {} vs {}\n", this->z.get_bins(), ctx.z.get_bins());
        return false;
    }

    return true;
}

bool basic_context::operator!=(const basic_context& ctx) { return !operator==(ctx); }

void basic_context::format_diff_axis()
{
    if (dim == dimension::NODIM) { throw dimension_error("Dimension not set"); }

    TString hunit = "1/";
    if (dimension::DIM1 <= dim) hunit += x.get_unit().Data();
    if (dimension::DIM2 <= dim) hunit += y.get_unit().Data();
    if (dimension::DIM3 == dim) hunit += z.get_unit().Data();

    UInt_t dim_cnt = 0;
    TString htitle;
    if (dimension::DIM3 == dim) dim_cnt = 3;
    if (dimension::DIM2 == dim) dim_cnt = 2;
    if (dimension::DIM1 == dim) dim_cnt = 1;

    if (dim > dimension::DIM1)
        htitle = TString::Format("d^{%d}%s/", dim_cnt, diff_var_name.Data());
    else
        htitle = TString::Format("d%s/", diff_var_name.Data());

    if (dim >= dimension::DIM1) htitle += TString("d") + x.get_label().Data();
    if (dim >= dimension::DIM2) htitle += TString("d") + y.get_label().Data();
    if (dim >= dimension::DIM3) htitle += TString("d") + z.get_label().Data();

    context_label = htitle;
    context_unit = hunit;

    if (context_unit.Length())
        context_axis_text = context_label + " [" + context_unit + "]";
    else
        context_axis_text = context_label;
}

TString basic_context::format_hist_axes(const char* title) const
{
    if (dim == dimension::NODIM) { throw dimension_error("Dimension not set"); }

    if (dimension::DIM3 == dim)
        return TString::Format("%s;%s%s%s;%s%s%s", title, x.get_label().Data(), x.format_unit().Data(),
                               y.get_label().Data(), y.format_unit().Data(), z.get_label().Data(),
                               z.format_unit().Data());
    else if (dimension::DIM2 == dim)
        return TString::Format("%s;%s%s;%s%s", title, x.get_label().Data(), x.format_unit().Data(),
                               y.get_label().Data(), y.format_unit().Data());
    else
        return TString::Format("%s;%s%s;Counts [aux]", title, x.get_label().Data(), x.format_unit().Data());
}

void basic_context::print() const
{
    fmt::print("Context: {}   Dimensions: {}\n", name.Data(), detail::dim_to_int(dim));
    fmt::print(" Name: {}   Hist name: {}   Dir Name: %s\n", name.Data(), hist_name.Data(), dir_name.Data());
    fmt::print(" Var name: {}\n", diff_var_name.Data());
    x.print();
    if (dim > midas::dimension::DIM1) y.print();
    if (dim > midas::dimension::DIM2) z.print();
}

}; // namespace midas

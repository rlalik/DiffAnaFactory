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

namespace midas
{

AxisCfg::AxisCfg() : bins(0), min(0), max(0), bins_arr(nullptr), delta(0.0), var(nullptr) {}

AxisCfg::AxisCfg(const AxisCfg& a) { *this = a; }

AxisCfg::~AxisCfg() noexcept {}

AxisCfg& AxisCfg::operator=(const AxisCfg& a)
{
    if (this == &a) return *this;

    label = a.label;
    unit = a.unit;

    bins = a.bins;
    min = a.min;
    max = a.max;
    bins_arr = a.bins_arr;
    delta = a.delta;
    var = a.var;

    return *this;
}

bool AxisCfg::operator==(const AxisCfg& ctx) { return (this->bins == ctx.bins); }

bool AxisCfg::operator!=(const AxisCfg& ctx) { return (this->bins != ctx.bins); }

TString AxisCfg::format_hist_string(const char* title, const char* ylabel) const
{
    return TString::Format("%s;%s;%s", title, format_string().Data(), ylabel);
}

TString AxisCfg::format_unit() const { return format_unit(unit); }

TString AxisCfg::format_string() const { return TString::Format("%s%s", label.Data(), format_unit().Data()); }

TString AxisCfg::format_unit(const char* unit) { return format_unit(TString(unit)); }

TString AxisCfg::format_unit(const TString& unit)
{
    TString funit;
    if (unit.Length()) funit = " [" + unit + "]";

    return funit;
}

void AxisCfg::print() const
{
    if (!bins_arr)
        printf(" Axis: %d bins in [ %f; %f ] range -- %s\n", bins, min, max, format_string().Data());
    else
    {
        TString buff;
        for (uint i = 0; i < bins; ++i)
        {
            buff += "| ";
            buff += bins_arr[i];
            buff += " ";
        }
        buff += "|";
        printf(" Axis: %d in %s -- %s\n", bins, buff.Data(), format_string().Data());
    }
}

} // namespace midas

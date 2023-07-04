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

#include <fmt/core.h>

namespace midas
{

axis_config::axis_config() : var(nullptr), bins(0), min(0), max(0), bins_arr(nullptr), delta(0.0) {}

auto axis_config::operator==(const axis_config& ctx) -> bool { return (this->bins == ctx.bins); }

auto axis_config::operator!=(const axis_config& ctx) -> bool { return (this->bins != ctx.bins); }

auto axis_config::format_hist_string(const char* title, const char* ylabel) const -> TString
{
    return TString::Format("%s;%s;%s", title, format_string().Data(), ylabel);
}

auto axis_config::format_unit() const -> TString { return format_unit(unit); }

auto axis_config::format_string() const -> TString
{
    return TString::Format("%s%s", label.Data(), format_unit().Data());
}

auto axis_config::format_unit(const char* unit) -> TString { return format_unit(TString(unit)); }

auto axis_config::format_unit(const TString& unit) -> TString
{
    TString funit;
    if (unit.Length()) funit = " [" + unit + "]";

    return funit;
}

auto axis_config::print() const -> void
{
    if (!bins_arr)
        fmt::print(" Axis: {} bins in [ {:g}; {:g} ] range -- {:s}\n", bins, min, max, format_string().Data());
    else
    {
        TString buff;
        for (auto i = 0; i < bins; ++i)
        {
            buff += "| ";
            buff += bins_arr[i];
            buff += " ";
        }
        buff += "|";
        fmt::print(" Axis: {} in {:s} -- {:s}\n", bins, buff.Data(), format_string().Data());
    }
}

auto axis_config::validate() const -> bool
{
    if (!bins) throw std::runtime_error("No axis bins specified");

    return true;
}

} // namespace midas

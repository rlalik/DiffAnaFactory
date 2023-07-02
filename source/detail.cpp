#include "detail.hpp"

#include <fmt/core.h>

#include <TList.h>

namespace midas
{
namespace detail
{

auto copyHistogram(TH1* src, TH1* dst, bool with_functions) -> bool
{
    if (!src or !dst) return false;

    auto bins_x = src->GetXaxis()->GetNbins();
    auto bins_y = src->GetYaxis()->GetNbins();

    for (auto x = 1; x <= bins_x; ++x)
    {
        for (auto y = 1; y <= bins_y; ++y)
        {
            auto bc = src->GetBinContent(x, y);
            auto be = src->GetBinError(x, y);
            dst->SetBinContent(x, y, bc);
            dst->SetBinError(x, y, be);
        }
    }

    if (!with_functions) return true;

    auto l = src->GetListOfFunctions();
    if (l->GetEntries())
    {
        dst->GetListOfFunctions()->Clear();
        for (int i = 0; i < l->GetEntries(); ++i)
        {
            dst->GetListOfFunctions()->Add(l->At(i)->Clone());
        }
    }

    return true;
}

auto jsonReadTStringKey(const Json::Value& jsondata, const char* key, TString& target) -> bool
{
    if (jsondata.isMember(key))
    {
        target = jsondata[key].asCString();
        fmt::print("    + {}: {}\n", key, target.Data());
        return true;
    }
    return false;
}

auto jsonReadIntKey(const Json::Value& jsondata, const char* key, int& target) -> bool
{
    if (jsondata.isMember(key))
    {
        target = jsondata[key].asInt();
        fmt::print("    + {}: {}\n", key, target);
        return true;
    }
    return false;
}

auto jsonReadUIntKey(const Json::Value& jsondata, const char* key, uint& target) -> bool
{
    if (jsondata.isMember(key))
    {
        target = jsondata[key].asUInt();
        fmt::print("    + {}: {}\n", key, target);
        return true;
    }
    return false;
}

auto jsonReadFloatKey(const Json::Value& jsondata, const char* key, float& target) -> bool
{
    if (jsondata.isMember(key))
    {
        target = jsondata[key].asFloat();
        fmt::print("    + {}: {}\n", key, target);
        return true;
    }
    return false;
}

auto jsonReadDoubleKey(const Json::Value& jsondata, const char* key, double& target) -> bool
{
    if (jsondata.isMember(key))
    {
        target = jsondata[key].asDouble();
        fmt::print("    + {}: {}\n", key, target);
        return true;
    }
    return false;
}

} // namespace detail
} // namespace midas

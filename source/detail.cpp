#include "detail.hpp"

#include <fmt/core.h>

#include <TH1.h>
#include <TList.h>

#include <cstring>
#include <linux/limits.h>
#include <sys/stat.h>

namespace midas
{
namespace detail
{

auto dim_to_int(dimension dim) -> int
{
    switch (dim)
    {
        case dimension::DIM1:
            return 1;
        case dimension::DIM2:
            return 2;
        case dimension::DIM3:
            return 3;
        case dimension::NODIM:
        default:
            return 0;
    }
}

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

auto find_json_file(const char* initial_path, const char* filename, int search_depth) -> json_file_info
{
    const size_t max_len = 1024 * 16;
    int depth_counter = 0;
    std::array<char, PATH_MAX + 1> resolv_name;
    std::array<char, PATH_MAX + 1> test_path;
    struct stat buffer;

    strncpy(test_path.data(), initial_path, max_len);

    json_file_info json_file;

    char* ret_val = 0;
    while (true)
    {
        ret_val = realpath(test_path.data(), resolv_name.data());
        if (!ret_val) break;

        std::string name = resolv_name.data();
        name += "/";
        name += filename;

        if (stat(name.c_str(), &buffer) == 0)
        {
            json_file.found = true;
            json_file.path = name;
            break;
        }

        strncpy(test_path.data(), resolv_name.data(), max_len);
        strncpy(test_path.data() + strlen(test_path.data()), "/..", 4);

        if (strcmp(resolv_name.data(), "/") == 0) break;

        ++depth_counter;
        if (search_depth >= 0 and (depth_counter > search_depth)) break;
    }

    if (json_file.found) fmt::print(" Found json config at {}\n", json_file.path.Data());

    return json_file;
}

auto json_read_TString_key(const Json::Value& jsondata, const char* key, TString& target) -> bool
{
    if (jsondata.isMember(key))
    {
        target = jsondata[key].asCString();
        fmt::print("    + {}: {}\n", key, target.Data());
        return true;
    }
    return false;
}

auto json_read_int_key(const Json::Value& jsondata, const char* key, int& target) -> bool
{
    if (jsondata.isMember(key))
    {
        target = jsondata[key].asInt();
        fmt::print("    + {}: {}\n", key, target);
        return true;
    }
    return false;
}

auto json_read_uint_key(const Json::Value& jsondata, const char* key, uint& target) -> bool
{
    if (jsondata.isMember(key))
    {
        target = jsondata[key].asUInt();
        fmt::print("    + {}: {}\n", key, target);
        return true;
    }
    return false;
}

auto json_read_float_key(const Json::Value& jsondata, const char* key, float& target) -> bool
{
    if (jsondata.isMember(key))
    {
        target = jsondata[key].asFloat();
        fmt::print("    + {}: {}\n", key, target);
        return true;
    }
    return false;
}

auto json_read_double_key(const Json::Value& jsondata, const char* key, double& target) -> bool
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

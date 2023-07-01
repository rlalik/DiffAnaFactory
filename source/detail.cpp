#include "detail.hpp"

#include <fmt/core.h>

namespace midas
{
namespace detail
{

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

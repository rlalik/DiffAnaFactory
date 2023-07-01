#ifndef MIDAS_DETAILS_H
#define MIDAS_DETAILS_H

#include "midas.hpp"

#include <TString.h>

#include <json/json.h>

namespace midas
{
namespace detail
{

constexpr auto dim_to_int(dimension dim) -> int {
    switch (dim) {
        case dimension::NODIM:
            return 0;
        case dimension::DIM1:
            return 1;
        case dimension::DIM2:
            return 2;
        case dimension::DIM3:
            return 3;
        default:
            return -1;
    }
}

auto jsonReadTStringKey(const Json::Value& jsondata, const char* key, TString& target) -> bool;
auto jsonReadIntKey(const Json::Value& jsondata, const char* key, int& target) -> bool;
auto jsonReadUIntKey(const Json::Value& jsondata, const char* key, uint& target) -> bool;
auto jsonReadFloatKey(const Json::Value& jsondata, const char* key, float& target) -> bool;
auto jsonReadDoubleKey(const Json::Value& jsondata, const char* key, double& target) -> bool;

} // namespace detail

} // namespace midas

#endif /* MIDAS_DETAILS_H*/

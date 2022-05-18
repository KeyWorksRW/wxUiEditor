/////////////////////////////////////////////////////////////////////////////
// Purpose:   Find std::string_view key in std::unordered_map
// Author:    https://en.cppreference.com/w/cpp/container/unordered_map/find
/////////////////////////////////////////////////////////////////////////////

// Example (only works with C++20 or later):
//
// std::unordered_map<std::string, std::string, str_view_hash, std::equal_to<>> my_map = {
//     { "foo", "bar" }
// };
//
// std::string_view name("foo");
//
// if (auto result = my_map.find(name); result != my_map.end())
// {
//     return result->second;

#if !(__cplusplus >= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L))
    #error "This method can only be used with a C++20 or later compiler"
#endif

#pragma once

#include <functional>  // for std::hash
#include <unordered_map>

struct str_view_hash
{
    using hash_type = std::hash<std::string_view>;
    using is_transparent = void;

    size_t operator()(const char* str) const { return hash_type {}(str); }
    size_t operator()(std::string_view str) const { return hash_type {}(str); }
    size_t operator()(std::string const& str) const { return hash_type {}(str); }
};

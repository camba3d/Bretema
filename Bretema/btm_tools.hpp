#pragma once

#include "btm_base.hpp"

#include <fstream>
#include <string>

namespace btm::fs
{

inline std::string read(std::string const &path)
{
    // Use ::ate to avoid '.seekg(0, std::ios::end)'
    auto file = std::ifstream { path, std::ios::ate | std::ios::binary };

    BTM_DEFER(file.close());

    if (!file.is_open())
    {
        BTM_ERRF("Issues opening shader: {}", path);
        return "";
    }

    std::string str;
    str.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(&str[0], str.size());

    return str;
}

}  // namespace btm::fs
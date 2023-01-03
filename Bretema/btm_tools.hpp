#pragma once

#include "btm_base.hpp"

#include <filesystem>
#include <fstream>
#include <string>

namespace btm::ds
{

class DeletionQueue
{
public:
    void push_back(std::function<void()> &&function) { deleteFuncs.push_back(function); }

    void flush()
    {
        for (auto it = deleteFuncs.rbegin(); it != deleteFuncs.rend(); it++)
        {
            (*it)();
        }
        deleteFuncs.clear();
    }

private:
    std::vector<std::function<void()>> deleteFuncs;
};

}  // namespace btm::ds

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
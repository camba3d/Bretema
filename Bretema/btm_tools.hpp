#pragma once

#include "btm_base.hpp"

#include <filesystem>
#include <fstream>

//=====================================
// DATA STRUCTURES & DATA STRUCTURES TOOLS
//=====================================
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

template<typename T>
auto view(std::vector<T> const &src, size_t len, size_t offset = 0) -> std::span<T const>
{
    size_t const fixedLen    = std::min(src.size(), len);
    size_t const fixedOffset = offset > fixedLen ? 0 : offset;
    return { src.data() + fixedOffset, fixedLen };
};

template<typename T>
auto view(T const *src, size_t len, size_t offset = 0) -> std::span<T const>
{
    size_t const fixedOffset = offset > len ? 0 : offset;
    return std::span<T const>(src + fixedOffset, len);
};

template<typename T>
auto merge(auto &dst, std::span<T> src) -> void
{
    if (!src.data() || src.empty())
        return;

    dst.reserve(dst.size() + src.size());
    dst.insert(dst.end(), src.begin(), src.end());
};

}  // namespace btm::ds

//=====================================
// FILES / FILESYSTEM TOOLS
//=====================================
namespace btm::fs
{

inline auto read(std::string const &path) -> std::string
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

//=====================================
// BINARY DATA TOOLS
//=====================================
namespace btm::bin
{

inline auto read(std::string const &path) -> std::vector<u8>
{
    std::ifstream   file { path, std::ios::binary };
    auto            fileBegin = std::istreambuf_iterator<char>(file);
    auto            fileEnd   = std::istreambuf_iterator<char>();
    std::vector<u8> raw { fileBegin, fileEnd };

    // if (raw.empty())
    // {
    //     BTM_ERRF("File '{}' empty or invalid", path);
    //     BTM_ASSERT(0);
    //     return {};
    // }

    return raw;
}

template<typename T>
auto checkMagic(std::span<const T> bin, std::vector<T> const &magic) -> bool
{
    if (magic.empty() || bin.size() < magic.size())
        return false;

    bool match = true;
    for (size_t i = 0; i < magic.size(); ++i)
    {
        match &= bin[i] == magic[i];
    }

    return match;
}

}  // namespace btm::bin

//
//
//
//
//
//
//
//
//
//

//=====================================
// PRINT HELPERS
//=====================================

// SPAN (DS::VIEW)
template<typename T>
struct fmt::formatter<std::span<T const>>
{
    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(std::span<T const> dataView, FormatContext &ctx) const -> decltype(ctx.out())
    {
        std::string s = "";
        for (auto const &v : dataView)
            s += BTM_FMT("{}, ", v);
        if (!dataView.empty())
            s.erase(s.end() - 2, s.end());

        return fmt::format_to(ctx.out(), "{}", s);
    }
};
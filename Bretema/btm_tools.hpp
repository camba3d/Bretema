#pragma once

#include "btm_base.hpp"

#include <filesystem>
#include <fstream>

namespace btm
{

//=====================================
// TIMER
//=====================================

template<typename TimeUnit, bool Scoped>
class Timer
{
    using Clock = std::chrono::high_resolution_clock;

public:
    Timer(std::string const &msg = "") : mMsg(msg) {}
    ~Timer()
    {
        if (Scoped)
            showElapsed();
    }

    inline void reset(std::string const &msg = "")
    {
        mMsg   = msg != "" ? msg : mMsg;
        mBegin = Clock::now();
    }

    inline i32 elapsed() const { return (i32)(std::chrono::duration_cast<TimeUnit>(Clock::now() - mBegin).count()); }

    inline std::string elapsedStr() const
    {
        std::string const unit = std::is_same_v<TimeUnit, std::chrono::nanoseconds> ? "ns" : "ms";
        return BTM_FMT("{} {}", elapsed(), unit);
    }

    inline void showElapsed() const { BTM_INFOF("[Timer] - {} : {}", mMsg, elapsedStr()); }

    inline void showElapsedAndReset()
    {
        showElapsed();
        reset();
    }

private:
    Clock::time_point mBegin = Clock::now();
    std::string       mMsg   = "";
};

using Timer_Ms       = Timer<std::chrono::milliseconds, false>;
using Timer_Ns       = Timer<std::chrono::nanoseconds, false>;
using ScopedTimer_Ms = Timer<std::chrono::milliseconds, true>;
using ScopedTimer_Ns = Timer<std::chrono::nanoseconds, true>;

//=====================================
// STRINGS
//=====================================
namespace str
{
inline std::string replace(std::string str, std::string const &from, std::string const &to, bool onlyFirstMatch = false)
{
    size_t pos = 0;
    while ((pos = str.find(from)) < str.size())
    {
        str.replace(pos, from.length(), to);

        if (onlyFirstMatch)
            break;
    }

    return str;
}

inline std::vector<std::string> split(const std::string &str, const std::string &delimeter)
{
    std::string              token;
    std::vector<std::string> splitted;
    size_t                   ini { 0 }, end { 0 };

    // Split and store the string body
    while ((end = str.find(delimeter, ini)) < str.size())
    {
        token = str.substr(ini, end - ini);
        ini   = end + delimeter.size();
        splitted.push_back(token);
    }

    // Store the string tail
    if (ini < str.size())
    {
        token = str.substr(ini);
        splitted.push_back(token);
    }

    return splitted;
}

}  // namespace str

//=====================================
// DATA STRUCTURES & DATA STRUCTURES TOOLS
//=====================================
namespace ds
{

class DeletionQueue
{
public:
    void add(std::function<void()> &&function) { deleteFuncs.push_back(function); }

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

template<typename T>  // pass len=-1 to use the vector size as len
auto view(std::vector<T> const &src, size_t len, size_t offset = 0) -> std::span<T const>
{
    size_t const fixedLen    = std::min(src.size(), len);
    size_t const fixedOffset = offset >= fixedLen ? 0 : offset;
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

}  // namespace ds

//=====================================
// FILES / FILESYSTEM TOOLS
//=====================================
namespace fs
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

}  // namespace fs

//=====================================
// BINARY DATA TOOLS
//=====================================
namespace bin
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

}  // namespace bin

//=====================================
// RUNTIME
//=====================================
namespace runtime
{
#ifdef WIN32
inline std::string exepath()
{
    char   result[MAX_PATH];
    size_t found;
    GetModuleFileName(NULL, result, MAX_PATH);
    found = std::string(result).find_last_of("\\");
    return str::replace(std::string(result).substr(0, found), "\\", "/");
}
#else
inline std::string exepath()
{
    char    result[PATH_MAX];
    size_t  found;
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    found         = std::string(result).find_last_of("/");
    // return(readlink(result).substr(0,found) + "/");
    return std::string(result, (count > 0) ? count : 0);
}
#endif

}  // namespace runtime

}  // namespace btm

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
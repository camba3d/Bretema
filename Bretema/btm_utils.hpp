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
// DATA STRUCTURES + TOOLS
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

/// @return execution path of the program
inline std::string exepath()
{
#ifdef WIN32
    char   result[MAX_PATH];
    size_t found;
    GetModuleFileName(NULL, result, MAX_PATH);
    found = std::string(result).find_last_of("\\");
    return str::replace(std::string(result).substr(0, found), "\\", "/");
#else
    char    result[PATH_MAX];
    size_t  found;
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    found         = std::string(result).find_last_of("/");
    // return(readlink(result).substr(0,found) + "/");
    return std::string(result, (count > 0) ? count : 0);
#endif
}

}  // namespace runtime

//=====================================
// MATHS
//=====================================
namespace math
{

/// @brief transform a value from a source range to an destination range
/// @param value the value to transform
/// @param srcMin source range min
/// @param srcMax source range max
/// @param dstMin destination range min
/// @param dstMax destination range max
/// @return transformed value
inline float map(float value, float srcMin, float srcMax, float dstMin, float dstMax)
{
    return dstMin + (dstMax - dstMin) * (value - srcMin) / (srcMax - srcMin);
}

/// @return a clamped angle in the range [0, 360)
inline float clampAngle(float angle)
{
    auto const turns = floorf(angle / 360.f);
    return angle - 360.f * turns;
}

// comparation helpers with fuzzy threshold
inline bool fuzzyCmp(float f1, float f2, float threshold = 0.01f)
{
    auto const diff = abs(f1 - f2);
    auto const isEq = diff <= threshold;
    return isEq;
}
inline bool fuzzyCmp(glm::vec2 const &v1, glm::vec2 const &v2, float t = 0.01f)
{
    return fuzzyCmp(v1.x, v2.x, t) && fuzzyCmp(v1.y, v2.y, t);
}
inline bool fuzzyCmp(glm::vec3 const &v1, glm::vec3 const &v2, float t = 0.01f)
{
    return fuzzyCmp(v1.x, v2.x, t) && fuzzyCmp(v1.y, v2.y, t) && fuzzyCmp(v1.z, v2.z, t);
}
inline bool fuzzyCmp(glm::vec4 const &v1, glm::vec4 const &v2, float t = 0.01f)
{
    return fuzzyCmp(v1.x, v2.x, t) && fuzzyCmp(v1.y, v2.y, t) && fuzzyCmp(v1.z, v2.z, t) && fuzzyCmp(v1.w, v2.w, t);
}

/// @tparam T glm::vec2/3/4
/// @param a first vector
/// @param b second vector
/// @param margin threshold to consider it aligned or not
/// @return True if its aligned, False otherwise.
template<typename T>
inline bool isAligned(T const &a, T const &b, float margin = 0.f)
{
    return abs(glm::dot(glm::normalize(a), glm::normalize(b))) >= (1.f - EPSILON - margin);
}

}  // namespace math

}  // namespace btm

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
        for (auto const &v : dataView) s += BTM_FMT("{}, ", v);
        if (!dataView.empty())
            s.erase(s.end() - 2, s.end());

        return fmt::format_to(ctx.out(), "{}", s);
    }
};
#pragma once

#include "base.hpp"

#include <filesystem>
#include <fstream>

namespace bm
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
        return BM_FMT("{} {}", elapsed(), unit);
    }

    inline void showElapsed() const { BM_INFOF("[Timer] - {} : {}", mMsg, elapsedStr()); }

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
    using fnType = std::function<void()>;

    void add(fnType const &fn) { mDestroyFuncs.push_back(fn); }

    void flush()
    {
        for (auto fnIt = mDestroyFuncs.rbegin(); fnIt != mDestroyFuncs.rend(); ++fnIt)
        {
            auto const &fn = *fnIt;
            if (fn)
            {
                fn();
            }
        }
        mDestroyFuncs.clear();
    }

private:
    std::vector<fnType> mDestroyFuncs;
};

template<typename T, size_t E = static_cast<size_t>(-1)>
using view = std::span<T const, E>;

// Raw Pointer : Const
template<typename T, size_t E = static_cast<size_t>(-1)>
view<T, E> make_view(T const *data, size_t len, size_t offset = 0)
{
    return { data + offset, len };
}

// Raw Pointer : No Const
template<typename T, size_t E = static_cast<size_t>(-1)>
view<T, E> make_view(T *data, size_t len, size_t offset = 0)
{
    return { const_cast<T const *>(data + offset), len };
}

// Vector : Const
template<typename T, size_t E = static_cast<size_t>(-1)>
view<T, E> make_view(std::vector<T> const &v, size_t len = E, size_t offset = 0)
{
    return { v.data() + offset, std::min(v.size(), len) };
}
// Vector : No Const
template<typename T, size_t E = static_cast<size_t>(-1)>
view<T, E> make_view(std::vector<T> &v, size_t len = E, size_t offset = 0)
{
    return { const_cast<T const *>(v.data() + offset), std::min(v.size(), len) };
}

template<typename T>
inline void merge(std::vector<T> &dst, view<T> src)
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
    BM_DEFER(file.close());

    if (!file.is_open())
    {
        BM_ERRF("Issues opening: {}", path);
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

inline std::vector<u8> read(std::string const &path)
{
    std::ifstream file { path, std::ios::binary };
    auto          fileBegin = std::istreambuf_iterator<char>(file);
    auto          fileEnd   = std::istreambuf_iterator<char>();
    return { fileBegin, fileEnd };
}

template<typename T>
inline bool checkMagic(ds::view<T> bin, std::vector<T> const &magic)
{
    if (magic.empty() || bin.size() < magic.size())
    {
        return false;
    }

    bool match = true;
    for (size_t i = 0; i < magic.size(); ++i)
    {
        match &= (bin[i] == magic[i]);
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

//=====================================
// COLORS
//=====================================
namespace color
{
glm::vec3 const Red          = { 1.f, 0.f, 0.f };
glm::vec3 const Green        = { 0.f, 1.f, 0.f };
glm::vec3 const Blue         = { 0.f, 0.f, 1.f };
glm::vec3 const Magenta      = { 1.f, 0.f, 1.f };
glm::vec3 const Yellow       = { 1.f, 1.f, 0.f };
glm::vec3 const Cyan         = { 0.f, 1.f, 1.f };
glm::vec3 const Lime         = { .5f, 1.f, 0.f };
glm::vec3 const Orange       = { 1.f, .3f, 0.f };
glm::vec3 const StrongYellow = { 1.f, .5f, 0.f };

inline glm::vec3 srgb_to_linear(glm::vec3 const &srgbColor)
{
    static glm::vec3 const invGamma { 2.23f };
    return glm::pow(srgbColor, invGamma);
}

// inline float srgb_to_linear(float f)
// {
//     return f < 0.04045f ? f * (1.0f / 12.92f) : glm::pow((f + 0.055f) * (float)(1.0 / (1.0 + 0.055)), 2.4f);
// }

// inline glm::vec3 srgb_to_linear(glm::vec3 const &c)
// {
//     return { srgb_to_linear(c.r), srgb_to_linear(c.g), srgb_to_linear(c.b) };
// }

inline float linear_to_srgb(float f)
{
    return f < 0.0031308f ? 12.92f * f : (1.0f + 0.055f) * glm::pow(f, 1.0f / 2.4f) - 0.055f;
}

inline glm::vec3 linear_to_srgb(glm::vec3 const &c)
{
    return { linear_to_srgb(c.r), linear_to_srgb(c.g), linear_to_srgb(c.b) };
}

inline glm::vec3 const hex_to_gl(std::string hexStr)
{
    //--- Safe input ------------------

    if (hexStr.empty())
    {
        return { 0.f, 0.f, 0.f };
    }

    if (hexStr.starts_with("#"))
    {
        hexStr.erase(0, 1);
    }

    if (hexStr.starts_with("0x"))
    {
        hexStr.erase(0, 2);
    }

    bool const oneCharPerChannel = hexStr.size() == 3 or hexStr.size() == 4;
    bool const twoCharPerChannel = hexStr.size() == 6 or hexStr.size() == 8;
    bool const hasAlpha          = hexStr.size() == 4 or hexStr.size() == 8;

    if (!oneCharPerChannel and !twoCharPerChannel)
    {
        return { 0.f, 0.f, 0.f };
    }

    if (oneCharPerChannel)
    {
        std::string const r = hexStr.substr(0, 1);
        std::string const g = hexStr.substr(1, 1);
        std::string const b = hexStr.substr(2, 1);
        std::string const a = hasAlpha ? hexStr.substr(4, 1) : "";

        hexStr = r + r + g + g + b + b + a + a;
    }

    hexStr = "0x" + hexStr;

    //--- Calculation -----------------

    u32 const hex = std::stoul(hexStr, nullptr, 16);

    u32 const r255 = ((hex >> 16) & 0xFF);
    u32 const g255 = ((hex >> 8) & 0xFF);
    u32 const b255 = ((hex) & 0xFF);

    float const r = r255 / 255.f;
    float const g = g255 / 255.f;
    float const b = b255 / 255.f;

    return srgb_to_linear({ r, g, b });
}

inline glm::vec4 const hex_to_gl(std::string const &hex_, float alpha)
{
    return glm::vec4(hex_to_gl(hex_), alpha);
}

}  // namespace color

}  // namespace bm

//
//
//

//=====================================
// PRINT HELPERS
//=====================================

// BM::DS::VIEW
template<typename T>
struct fmt::formatter<bm::ds::view<T>>
{
    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(bm::ds::view<T> view, FormatContext &ctx) const -> decltype(ctx.out())
    {
        std::string s = "";
        for (auto const &v : view) s += BM_FMT("{}, ", v);
        if (!view.empty())
            s.erase(s.end() - 2, s.end());

        return fmt::format_to(ctx.out(), "{}", s);
    }
};
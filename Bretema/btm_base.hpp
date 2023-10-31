#pragma once

//===========================
//= INCLUDES
//===========================

//--- C++ -------------------------------------------------
#include <cmath>
#include <limits>
#include <cstdint>
#include <algorithm>
#include <fstream>
#include <memory>
#include <functional>
#include <map>
#include <set>
#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <typeinfo>
#include <source_location>
//---------------------------------------------------------

//--- GLM -------------------------------------------------
// #define GLM_FORCE_SSE
// #define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_INLINE
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_SILENT_WARNINGS
#define GLM_FORCE_RADIANS
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vec_swizzle.hpp>
#include <glm/gtx/vector_angle.hpp>
//---------------------------------------------------------

//--- FMT -------------------------------------------------
#include <fmt/format.h>
//---------------------------------------------------------

//--- SPAN ------------------------------------------------
#if 1
#    include <span>
#else
#    define TCB_SPAN_NAMESPACE_NAME std
#    include "span.hpp"
#endif

//---------------------------------------------------------

//===========================
//= GLOBAL
//===========================

//--- C++ TYPES ALIASES -----------------------------------
// Numbers
using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;
// Smart Pointers
template<typename T>
using sPtr = std::shared_ptr<T>;
#define sNew std::make_shared
template<typename T>
using uPtr = std::unique_ptr<T>;
#define uNew std::make_unique
// Data Structures
template<typename K, typename V>
using umap = std::unordered_map<K, V>;
template<typename T>
using uset = std::unordered_set<T>;
//---------------------------------------------------------

//--- C++ MACROS ALIASES ----------------------------------
#define MBU           [[maybe_unused]]
#define NDSC          [[nodiscard]]
#define BTM_UNUSED(x) (void)x
#define BTM_BIT(x)    (1 << x)
#define BTM_BIND(fn)  [this](auto &&...args) { return this->fn(args...); }
//---------------------------------------------------------

//--- GLOBAL STR HELPERS ----------------------------------
/// TypeName to String
template<typename T>
inline std::string BTM_STR_TYPE()
{
    return typeid(T).name();
}
/// Ptr to String
#define BTM_STR_PTR(p) fmt::format("{}", fmt::ptr(p))
/// Trim
auto const BTM_STR_TRIM = [](auto const &s, i32 nChars) -> std::string_view
{
    std::string_view const sv = s;
    if (nChars < 1)
        return sv;
    size_t const n = static_cast<size_t>(nChars);
    return sv.substr(sv.length() >= n ? sv.length() - n : 0);
};
//---------------------------------------------------------

//--- LOG (without format) --------------------------------
// #define BTM_FULL_LENGTH_LOG
#ifdef BTM_FULL_LENGTH_LOG
#    define BTM_INFO(msg) fmt::print("[I] - ({}:{})\n → {}\n", __FILE__, __LINE__, msg)
#    define BTM_WARN(msg) fmt::print("[W] - ({}:{})\n → {}\n", __FILE__, __LINE__, msg)
#    define BTM_ERR(msg)  fmt::print("[E] - ({}:{})\n → {}\n", __FILE__, __LINE__, msg)
#else
#    define BTM_INFO(msg) fmt::print("[I] (...{}:{}) - {}\n", BTM_STR_TRIM(__FILE__, 20), __LINE__, msg)
#    define BTM_WARN(msg) fmt::print("[W] (...{}:{}) - {}\n", BTM_STR_TRIM(__FILE__, 20), __LINE__, msg)
#    define BTM_ERR(msg)  fmt::print("[E] (...{}:{}) - {}\n", BTM_STR_TRIM(__FILE__, 20), __LINE__, msg)
#endif
//---------------------------------------------------------

//--- LOG (with format) -----------------------------------
#define BTM_FMT(msg, ...)   fmt::format(msg, __VA_ARGS__)
#define BTM_INFOF(msg, ...) BTM_INFO(BTM_FMT(msg, __VA_ARGS__))
#define BTM_WARNF(msg, ...) BTM_WARN(BTM_FMT(msg, __VA_ARGS__))
#define BTM_ERRF(msg, ...)  BTM_ERR(BTM_FMT(msg, __VA_ARGS__))
//---------------------------------------------------------

//--- TRACE ----------------------------------------------
inline void BTM_TRACE(std::source_location const &sl = std::source_location::current())
{
    auto const name = sl.function_name();
    fmt::print("[*] - {}\n", name);
};
//---------------------------------------------------------

// clang-format off

//--- ASSERT ----------------------------------------------
#ifndef NDEBUG
#    define BTM_ASSERT(cond) assert(cond)
#    define BTM_ASSERT_X(cond, msg) do { if (!(cond)) { BTM_ERR(msg); assert(cond); } } while (0)
#else
#    define BTM_ASSERT(cond)
#    define BTM_ASSERT_X(cond, msg)
#endif
//---------------------------------------------------------

//--- ABORT -----------------------------------------------
#define BTM_ABORT(msg) do { BTM_ERR(msg); abort(); } while (0)
#define BTM_ABORTF(msg, ...) do { BTM_ERRF(msg, __VA_ARGS__); abort(); } while (0)
#define BTM_ABORT_IF(cond, msg) do { if (cond) BTM_ABORTF("{} --> {}", #cond, msg); } while (0)
#define BTM_ABORTF_IF(cond, msg, ...) do { if (cond) BTM_ABORTF("{} --> {}", #cond, fmt::format(msg, __VA_ARGS__)); } while (0)
//---------------------------------------------------------

//--- CONCAT ----------------------------------------------
// Put together two parameters passed to the macro, its the
//   its the right way to glue things like varName_##__LINE__
//   where __LINE__ is also a macro that extracts the line number
#define detail_BTM_CONCAT(a, b) a##b
#define BTM_CONCAT(a, b) detail_BTM_CONCAT(a, b)
//---------------------------------------------------------

//--- DEFER -----------------------------------------------
// This calls the code right after the scope-ends
//   allowing us to call things like file.close(), right after file.open(),
//   to be sure that we don't forget to call it at the end of the function
//   and ensure that it's called even if an exception is thrown.
#define detail_BTM_DEFER0 std::unique_ptr<void, std::function<void (void *)>>
#define detail_BTM_DEFER1 []() { static int a=0; return &a; }
#define BTM_DEFER(fn) auto BTM_CONCAT(defer_,__LINE__) = detail_BTM_DEFER0( detail_BTM_DEFER1(), [&](void *) { fn; } )
//---------------------------------------------------------

// clang-format on

//===========================
//= FORCE DISCRETE GPU
//===========================

#if defined(_WIN64) && defined(_WIN32) && defined(_MSC_VER)
#    define BTM_FORCE_DISCRETE_GPU                                                        \
        extern "C"                                                                        \
        {                                                                                 \
            _declspec(dllexport) DWORD NvOptimusEnablement                  = 0x00000001; \
            _declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001; \
        }
#elif defined(_WIN64) && defined(_WIN32)
#    define BTM_FORCE_DISCRETE_GPU                                                   \
        extern "C"                                                                   \
        {                                                                            \
            __attribute__((dllexport)) int NvOptimusEnablement                  = 1; \
            __attribute__((dllexport)) int AmdPowerXpressRequestHighPerformance = 1; \
        }
#endif

//===========================
//= BRETEMA CONSTS
//===========================

namespace btm
{

// Axis
inline glm::vec3 const RIGHT = { 1, 0, 0 };
inline glm::vec3 const UP    = { 0, 1, 0 };
inline glm::vec3 const FRONT = { 0, 0, 1 };

// Flip axis
inline glm::vec3 const FLIP_X = { -1, 1, 1 };
inline glm::vec3 const FLIP_Y = { 1, -1, 1 };
inline glm::vec3 const FLIP_Z = { 1, 1, -1 };

// Inf
inline float const     INF  = std::numeric_limits<float>::infinity();
inline glm::vec2 const INF2 = { INF, INF };
inline glm::vec3 const INF3 = { INF, INF, INF };
inline glm::vec4 const INF4 = { INF, INF, INF, INF };

// Zero
inline float const     ZERO  = 0.f;
inline glm::vec2 const ZERO2 = { 0.f, 0.f };
inline glm::vec3 const ZERO3 = { 0.f, 0.f, 0.f };
inline glm::vec4 const ZERO4 = { 0.f, 0.f, 0.f, 0.f };

// One
inline float const     ONE  = 1.f;
inline glm::vec2 const ONE2 = { 1.f, 1.f };
inline glm::vec3 const ONE3 = { 1.f, 1.f, 1.f };
inline glm::vec4 const ONE4 = { 1.f, 1.f, 1.f, 1.f };

// Vec2 Permutations
inline glm::vec2 const X2  = { 1, 0 };
inline glm::vec2 const Y2  = { 0, 1 };
inline glm::vec2 const XY2 = { 1, 1 };

// Vec3 Permutations
inline glm::vec3 const X3   = { 1, 0, 0 };
inline glm::vec3 const Y3   = { 0, 1, 0 };
inline glm::vec3 const Z3   = { 0, 0, 1 };
inline glm::vec3 const YZ3  = { 0, 1, 1 };
inline glm::vec3 const XZ3  = { 1, 0, 1 };
inline glm::vec3 const XY3  = { 1, 1, 0 };
inline glm::vec3 const XYZ3 = { 1, 1, 1 };

// Vec4 Permutations
inline glm::vec4 const X4    = { 1, 0, 0, 0 };
inline glm::vec4 const Y4    = { 0, 1, 0, 0 };
inline glm::vec4 const Z4    = { 0, 0, 1, 0 };
inline glm::vec4 const YZ4   = { 0, 1, 1, 0 };
inline glm::vec4 const XZ4   = { 1, 0, 1, 0 };
inline glm::vec4 const XY4   = { 1, 1, 0, 0 };
inline glm::vec4 const XW4   = { 1, 0, 0, 1 };
inline glm::vec4 const YW4   = { 0, 1, 0, 1 };
inline glm::vec4 const ZW4   = { 0, 0, 1, 1 };
inline glm::vec4 const XYZ4  = { 1, 1, 1, 0 };
inline glm::vec4 const XYW4  = { 1, 1, 0, 1 };
inline glm::vec4 const XZW4  = { 1, 0, 1, 1 };
inline glm::vec4 const YZW4  = { 0, 1, 1, 1 };
inline glm::vec4 const XYZW4 = { 1, 1, 1, 1 };

// Math consts
inline float const PI           = 3.14159265359f;
inline float const TAU          = 2.f * PI;
inline float const HALF_PI      = PI * 0.5f;
inline float const SOFT_EPSILON = 0.00001f;
inline float const EPSILON      = std::numeric_limits<float>::epsilon();

// Limits
inline i32 constexpr MAX_PRINT_DECIMALS = 3;

}  // namespace btm

//
//
//

//=====================================
// PRINT HELPERS
//=====================================

// GLM
template<>
struct fmt::formatter<glm::vec2>
{
    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const glm::vec2 &v, FormatContext &ctx) const -> decltype(ctx.out())
    {
        auto const &fp = btm::MAX_PRINT_DECIMALS;
        return fmt::format_to(ctx.out(), "({:.{}f},{:.{}f})", v.x, fp, v.y, fp);
    }
};
template<>
struct fmt::formatter<glm::vec3>
{
    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const glm::vec3 &v, FormatContext &ctx) const -> decltype(ctx.out())
    {
        auto const &fp = btm::MAX_PRINT_DECIMALS;
        return fmt::format_to(ctx.out(), "({:.{}f},{:.{}f},{:.{}f})", v.x, fp, v.y, fp, v.z, fp);
    }
};
template<>
struct fmt::formatter<glm::vec4>
{
    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const glm::vec4 &v, FormatContext &ctx) const -> decltype(ctx.out())
    {
        auto const &fp = btm::MAX_PRINT_DECIMALS;
        return fmt::format_to(ctx.out(), "({:.{}f},{:.{}f},{:.{}f},{:.{}f})", v.x, fp, v.y, fp, v.z, fp, v.w, fp);
    }
};
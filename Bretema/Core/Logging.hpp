#pragma once

#include "Std.hpp"
#include <source_location>
#include <fmt/format.h>

//=========================================================
// Helpers
//=========================================================

inline std::string bmStrTrim(std::string s, i32 n)
{
    i32 const sl = s.length();
    return n < 1 ? s : s.substr(sl >= n ? sl - n : 0);
};

inline std::string bmUnixPath(std::string str)
{
    static std::string const from = "\\";
    static std::string const to   = "/";

    size_t pos = 0;
    while ((pos = str.find(from)) < str.size())
    {
        str.replace(pos, from.length(), to);
    }

    return str;
}

//=========================================================

#define BM_FULL_LENGTH_LOG

//--- LOG (without format) --------------------------------

#ifdef BM_FULL_LENGTH_LOG
#    define BM_INFO(msg) fmt::print("[I] {}:{}\n[=]---→ {}\n", bmUnixPath(__FILE__), __LINE__, msg)
#    define BM_WARN(msg) fmt::print("[W] {}:{}\n[=]---→ {}\n", bmUnixPath(__FILE__), __LINE__, msg)
#    define BM_ERR(msg)  fmt::print("[E] {}:{}\n[=]---→ {}\n", bmUnixPath(__FILE__), __LINE__, msg)
#else
#    define BM_INFO(msg) fmt::print("[I] (...{}:{}) - {}\n", bmStrTrim(__FILE__, 20), __LINE__, msg)
#    define BM_WARN(msg) fmt::print("[W] (...{}:{}) - {}\n", bmStrTrim(__FILE__, 20), __LINE__, msg)
#    define BM_ERR(msg)  fmt::print("[E] (...{}:{}) - {}\n", bmStrTrim(__FILE__, 20), __LINE__, msg)
#endif

//

//--- LOG (with format) -----------------------------------

#define BM_FMT(msg, ...)   fmt::format(msg, __VA_ARGS__)
#define BM_INFOF(msg, ...) BM_INFO(BM_FMT(msg, __VA_ARGS__))
#define BM_WARNF(msg, ...) BM_WARN(BM_FMT(msg, __VA_ARGS__))
#define BM_ERRF(msg, ...)  BM_ERR(BM_FMT(msg, __VA_ARGS__))

//

//--- TRACE ----------------------------------------------

inline void BM_TRACE(std::source_location const &sl = std::source_location::current())
{
    auto const funcname = sl.function_name();
    auto const filename = sl.file_name();
    auto const line     = sl.line();
    fmt::print("[*] {}:{} ( {} )\n", filename, line, funcname);
};

//
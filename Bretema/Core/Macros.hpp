#pragma once

#include <string>

//--- ALIASES ---------------------------------------------

#define MBU          [[maybe_unused]]
#define NDSC         [[nodiscard]]
#define BM_UNUSED(x) (void)x
#define BM_BIT(x)    (1 << x)
#define BM_BIND(fn)  [this](auto &&...args) { return this->fn(args...); }

//

//--- CONCAT ----------------------------------------------

// Put together two parameters passed to the macro, its the
//   its the right way to glue things like varName_##__LINE__
//   where __LINE__ is also a macro that extracts the line number

// clang-format off
#define detail_BM_CONCAT(a, b) a##b
#define BM_CONCAT(a, b) detail_BM_CONCAT(a, b)
// clang-format on

//

//--- DEFER -----------------------------------------------

// This calls the code right after the scope-ends
//   allowing us to call things like file.close(), right after file.open(),
//   to be sure that we don't forget to call it at the end of the function
//   and ensure that it's called even if an exception is thrown.

// clang-format off
#define detail_BM_DEFER0 std::unique_ptr<void, std::function<void (void *)>>
#define detail_BM_DEFER1 []() { static int a=0; return &a; }
#define BM_DEFER(fn) auto BM_CONCAT(defer_,__LINE__) = detail_BM_DEFER0( detail_BM_DEFER1(), [&](void *) { fn; } )
// clang-format on

//

//--- FORCE DISCRETE GPU ----------------------------------

#if defined(_WIN64) && defined(_WIN32) && defined(_MSC_VER)
#    define BM_FORCE_DISCRETE_GPU                                                         \
        extern "C"                                                                        \
        {                                                                                 \
            _declspec(dllexport) DWORD NvOptimusEnablement                  = 0x00000001; \
            _declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001; \
        }
#elif defined(_WIN64) && defined(_WIN32)
#    define BM_FORCE_DISCRETE_GPU                                                    \
        extern "C"                                                                   \
        {                                                                            \
            __attribute__((dllexport)) int NvOptimusEnablement                  = 1; \
            __attribute__((dllexport)) int AmdPowerXpressRequestHighPerformance = 1; \
        }
#endif

//

//--- STRINGS ---------------------------------------------

template<typename T>
inline std::string BM_STR_TYPE()
{
    return typeid(T).name();
}
#define BM_STR_TYPE(obj) BM_STR_TYPE<decltype(obj)>()
#define BM_STR_PTR(p)    fmt::format("{}", fmt::ptr(p))

//
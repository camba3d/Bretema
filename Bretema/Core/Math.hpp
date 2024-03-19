#pragma once

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

#include "Logging.hpp"

template<glm::length_t C>
struct fmt::formatter<glm::vec<C, float, glm::defaultp>>
{
    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const glm::vec<C, float, glm::defaultp> &v, FormatContext &ctx) const -> decltype(ctx.out())
    {
        std::string s = "(";
        for (glm::length_t i = 0; i < C; ++i)
        {
            s += fmt::format("{:.{}f},", v[i], bm::MAX_PRINT_DECIMALS);
        }
        s.erase(s.end() - 1, s.end());
        s += ")";
        return fmt::format_to(ctx.out(), "{}", s);
    }
};
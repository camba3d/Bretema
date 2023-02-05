#pragma once

/* NOTE:
 *
 *  Include the <vk/dx/gl/mt/wg>-Renderer files before
 *  the BaseRenderer like :
 *
 *    #include "vk_renderer.hpp"
 *    #include "../btm_renderer.hpp"
 *
 */

#include "btm_base.hpp"
#include "btm_window.hpp"

namespace btm
{

//===========================
//= TYPEDEFS
//===========================

using UUID = std::string;

//===========================
//= CONTANTS
//===========================

glm::vec3 constexpr RIGHT = { 1, 0, 0 };
glm::vec3 constexpr UP    = { 0, 1, 0 };
glm::vec3 constexpr FRONT = { 0, 0, 1 };

//===========================
//= ENUMS
//===========================

enum struct Cull
{
    NONE,
    CW,
    CCW,
};

enum struct Compare
{
    NONE,
    LESS,
    LESS_EQ,
    GREAT,
    GREAT_EQ
};
using Depth = Compare;

enum struct Samples
{
    _1,
    _2,
    _4,
    _8,
    _16,
    _32,
    _64,
};

//===========================
//= HELPER STRUCTS
//===========================

struct Area2D
{
    glm::vec2 off = {};  // Rect's init-point
    glm::vec2 ext = {};  // Rect's end-point
};

struct Area3D
{
    glm::vec3 off = {};  // Cube's init-point
    glm::vec3 ext = {};  // Cube's end-point
};

//===========================
//= TYPES
//===========================

struct Mesh
{
    std::string name = "";

    struct Instance
    {
        glm::mat4 transform;
        glm::vec4 color;
    };

    // Indices
    std::vector<uint32_t> indices;

    // Vertices
    std::vector<float> positions = {};  // 3:xyz
    std::vector<float> uvs0      = {};  // 2:xy
    std::vector<float> normals   = {};  // 3:xyz
    std::vector<float> tangents  = {};  // 4 : xyzw - XYZ:normalized, W:-1|+1 (handeness)

    // Instances
    std::vector<Instance> instances;
};

//===========================
//= BASE RENDERER
//===========================

class BaseRenderer
{
public:
    inline static constexpr int32_t sInFlight = 3;

    BaseRenderer(Ref<btm::Window> window);
    virtual ~BaseRenderer() = default;

    virtual void update() { BTM_ASSERT_X(0, "Cast-back to non-base renderer!"); }
    virtual void draw() { BTM_ASSERT_X(0, "Cast-back to non-base renderer!"); }
    virtual void cleanup() { BTM_ASSERT_X(0, "Cast-back to non-base renderer!"); }

    inline bool isInitialized() { return mInit; }

protected:
    inline void markAsInit() { mInit = true; }

    int32_t mFrameNumber = 0;

    bool      mInit         = false;
    void     *mWindowHandle = nullptr;
    glm::vec2 mViewportSize = { 1280, 720 };
};

//===========================
//= TOOLS
//===========================

std::vector<Mesh> parseGltf(std::string const &filepath);
// std::vector<Mesh> parseGltf(std::uint8_t const *data, std::size_t size);

}  // namespace btm

//.............................................................................
//.............................................................................
//.............................................................................
//.............................................................................
//.............................................................................

//=====================================
// PRINT HELPERS
//=====================================

template<>
struct fmt::formatter<btm::Mesh>
{
    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const btm::Mesh &mesh, FormatContext &ctx) const -> decltype(ctx.out())
    {
        auto const x = mesh.positions.at(0);
        auto const y = mesh.positions.at(1);
        auto const z = mesh.positions.at(2);
        return fmt::format_to(ctx.out(), "{} : ({},{},{})", mesh.name, x, y, z);
    }
};
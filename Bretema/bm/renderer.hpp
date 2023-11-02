#pragma once

#include "base.hpp"
#include "window.hpp"

#include "camera.hpp"

namespace bm
{

//===========================
//= TYPEDEFS
//===========================

using UUID = std::string;

//===========================
//= ENUMS
//===========================

enum struct Cull
{
    NONE,
    CW,   // Front
    CCW,  // Back
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
//= AUX STRUCTS
//===========================

struct Area2D
{
    glm::vec2     off = {};  // Rect's init-point
    glm::vec2     ext = {};  // Rect's end-point
    float         w() { return (ext - off).x; }
    float         h() { return (ext - off).y; }
    glm::vec4     asRect() { return { off, ext - off }; }
    static Area2D fromInitEnd(glm::vec2 const &init, glm::vec2 const &end) { return { init, end }; }
    static Area2D fromInitSize(glm::vec2 const &init, glm::vec2 const &size) { return { init, init + size }; }
};

struct Area3D
{
    glm::vec3     off = {};  // Cube's init-point
    glm::vec3     ext = {};  // Cube's end-point
    float         w() { return (ext - off).x; }
    float         h() { return (ext - off).y; }
    float         d() { return (ext - off).z; }
    static Area3D fromInitEnd(glm::vec3 const &init, glm::vec3 const &end) { return { init, end }; }
    static Area3D fromInitSize(glm::vec3 const &init, glm::vec3 const &size) { return { init, init + size }; }
};

//===========================
//= TYPES
//===========================

using MeshIndices = std::vector<u16>;
struct Mesh
{
    std::string name = "";

    struct Vertex
    {
        glm::vec3 pos     = {};  // 3:xyz
        glm::vec2 uv0     = {};  // 2:xy
        glm::vec3 normal  = {};  // 3:xyz
        glm::vec4 tangent = {};  // 4 : xyzw - XYZ:normalized, W:-1|+1 (handeness)
    };

    struct Instance
    {
        glm::mat4 transform;
        glm::vec4 color;
    };

    MeshIndices           indices;
    std::vector<Vertex>   vertices;
    std::vector<Instance> instances;
};
using Vertices      = std::vector<Mesh::Vertex>;
using Instances     = std::vector<Mesh::Instance>;
using MeshVertices  = Vertices;
using MeshInstances = Instances;
using MeshGroup     = std::vector<Mesh>;

struct Material
{
};

//===========================
//= BASE RENDERER
//===========================

class BaseRenderer
{
public:
    inline static constexpr i32 sInFlight = 3;

    // LIFETIME
    BaseRenderer(sPtr<bm::Window> window);
    virtual ~BaseRenderer() = default;

    // PROPS
    inline bool isInitialized() { return mInit; }

    inline float winW()
    {
        BM_ASSERT(mWindow);
        return mWindow ? mWindow->size().x : 1;
    }
    inline float winH()
    {
        BM_ASSERT(mWindow);
        return mWindow ? mWindow->size().y : 1;
    }
    inline float w() { return mSize.x; }
    inline float h() { return mSize.y; }

    // ACTIONS
    virtual void update()                = 0;
    virtual void draw(Camera const &cam) = 0;
    virtual void cleanup()               = 0;

    bool windowSizeMatch() { return mSize == winSize(); }
    void windowSizeSync() { mSize = winSize(); }

protected:
    inline void markAsInit() { mInit = true; }

    glm::vec2 winSize() { return mWindow ? mWindow->size() : ONE2; }

    bool             mInit        = false;
    i32              mFrameNumber = 0;
    glm::vec2        mSize        = { 1280, 720 };
    sPtr<bm::Window> mWindow      = nullptr;
};

//===========================
//= TOOLS
//===========================

MeshGroup parseGltf(std::string const &filepath);
MeshGroup parseGltf(ds::view<u8> bin, std::string name = "");

}  // namespace bm

//
//
//

//=====================================
// PRINT HELPERS
//=====================================

// Mesh
template<>
struct fmt::formatter<bm::Mesh>
{
    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const bm::Mesh &mesh, FormatContext &ctx) const -> decltype(ctx.out())
    {
        std::string meshStr = "INDICES:\n";
        for (auto const &i : mesh.indices) meshStr += BM_FMT("{}, ", i);

        if (!mesh.indices.empty())
            meshStr.erase(meshStr.end() - 2, meshStr.end());

        meshStr += "\nATTRIBUTES (pos / uv0 / normal / tangent) :\n";

        size_t vn = 0;
        for (auto const &v : mesh.vertices) meshStr += BM_FMT("{}: {} / {} / {} / {}", vn++, v.pos, v.uv0, v.normal, v.tangent) + "\n";

        return fmt::format_to(ctx.out(), "\n{}\n......\n{}", mesh.name, meshStr);
    }
};

// MeshGroup
template<>
struct fmt::formatter<bm::MeshGroup>
{
    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const bm::MeshGroup &meshGroup, FormatContext &ctx) const -> decltype(ctx.out())
    {
        std::string meshGroupStr = "";
        for (auto const &mesh : meshGroup) meshGroupStr += BM_FMT("{}", mesh) + "\n";

        return fmt::format_to(ctx.out(), "\nMESHGROUP\n--------\n{}", meshGroupStr);
    }
};

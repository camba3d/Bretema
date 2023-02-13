#include "btm_renderer.hpp"
#include "btm_tools.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION  // optional. disable exception handling.
#include "tiny_gltf.h"

namespace btm
{

BaseRenderer::BaseRenderer(Ref<btm::Window> window)
{
    mWindowHandle = window->handle();
    mViewportSize = window->size();

    BTM_ASSERT_X(mWindowHandle, "Invalid window handle");
    BTM_ASSERT_X(mViewportSize.x > 0 && mViewportSize.y > 0, "Invalid viewport size");
}

// Load a glTF scene from a file path

auto getVec2   = [](float const *d, int i) { return glm::vec2 { d[i + 0], d[i + 1] }; };
auto getVec3   = [](float const *d, int i) { return glm::vec3 { d[i + 0], d[i + 1], d[i + 2] }; };
auto getVec4   = [](float const *d, int i) { return glm::vec4 { d[i + 0], d[i + 1], d[i + 2], d[i + 3] }; };
auto getFloats = [](float const *d, int i) { return glm::vec4 { d[i + 0], d[i + 1], d[i + 2], d[i + 3] }; };

template<typename T>
std::tuple<T const *, u32> digestMeshProp(tinygltf::Model const &model, int32_t accessorIdx)
{
    T const *data  = nullptr;
    u32      count = 0;

    if (accessorIdx < 0)
        return std::make_tuple(data, count);

    auto const &accessor   = model.accessors[accessorIdx];
    auto const &bufferView = model.bufferViews[accessor.bufferView];
    auto const &buffer     = model.buffers[bufferView.buffer];
    auto const  offset     = bufferView.byteOffset + accessor.byteOffset;

    data  = reinterpret_cast<T const *>(&buffer.data[offset]);
    count = static_cast<u32>(accessor.count);

    return std::make_tuple(data, count);
}

std::vector<Mesh> parseGltf(tinygltf::TinyGLTF const &ctx, tinygltf::Model const &model)
{
    std::vector<Mesh> meshes;

    for (auto const &mesh : model.meshes)
    {
        Mesh outMesh;
        outMesh.name = mesh.name;

        for (const auto &primitive : mesh.primitives)
        {
            if (primitive.mode != TINYGLTF_MODE_TRIANGLES)
                continue;

            // INDICES
            auto [data, count] = digestMeshProp<u32>(model, primitive.indices);
            outMesh.indices.assign(data, data + count);

            // ATTRIBUTES
            static std::array const sAttributes = {
                std::make_tuple(Mesh::Pos, 3, "POSITION"),
                std::make_tuple(Mesh::UV0, 2, "TEXCOORD_Ø"),
                std::make_tuple(Mesh::Normal, 3, "NORMAL"),
                std::make_tuple(Mesh::Tangent, 4, "TANGENT"),
            };

            for (auto const &[type, components, name] : sAttributes)
            {
                int32_t accessorIdx = primitive.attributes.count(name) > 0 ? primitive.attributes.at(name) : -1;
                auto [data, count]  = digestMeshProp<float>(model, accessorIdx);

                size_t const numVertices = count / components;
                outMesh.vertices.resize(numVertices);

                for (size_t i = 0; i < numVertices; ++i)
                {
                    size_t curr = components * i;
                    auto  &v    = outMesh.vertices[i];

                    switch (type)
                    {
                        case Mesh::Pos: v.pos = getVec3(data, curr); break;
                        case Mesh::UV0: v.uv0 = getVec2(data, curr); break;
                        case Mesh::Normal: v.normal = getVec3(data, curr); break;
                        case Mesh::Tangent: v.tangent = getVec4(data, curr); break;
                        default: break;
                    }
                }
            }
        }

        meshes.push_back(outMesh);
    }

    BTM_INFOF("END ParseGltf => {}", meshes);

    return meshes;
}

std::vector<Mesh> parseGltf(std::string const &filepath)
{
    auto const bin   = btm::bin::read(filepath);
    auto const isBin = btm::bin::checkMagic(std::span(bin.data(), 4), { 'g', 'l', 'T', 'F' });

    if (bin.empty())
        return {};

    tinygltf::TinyGLTF ctx;  //@dani : make this part of the renderer? or "global" to only init it once...
    tinygltf::Model    model;
    std::string        err, warn;

    bool const ok = !isBin ? ctx.LoadASCIIFromFile(&model, &err, &warn, filepath)
                           : ctx.LoadBinaryFromMemory(&model, &err, &warn, bin.data(), bin.size());

    if (!err.empty())
        BTM_ERRF("Loading GLTF '{}' : {}", filepath, err);
    if (!warn.empty())
        BTM_WARNF("Loading GLTF '{}' : {}", filepath, warn);
    if (!ok)
        BTM_WARNF("Loading GLTF '{}' : Undefined error", filepath);
    if (!err.empty() or !warn.empty() or !ok)
        return {};

    return parseGltf(ctx, model);
}

std::vector<Mesh> parseGltf(std::span<u8> bin)
{
    tinygltf::TinyGLTF ctx;  //@dani : make this part of the renderer? or "global" to only init it once...
    tinygltf::Model    model;
    std::string        err, warn;

    bool const ok = ctx.LoadBinaryFromMemory(&model, &err, &warn, bin.data(), bin.size());

    if (!err.empty())
        BTM_ERRF("Loading Bin GLTF : {}", err);
    if (!warn.empty())
        BTM_WARNF("Loading Bin GLTF : {}", warn);
    if (!ok)
        BTM_WARN("Loading Bin GLTF : Undefined error");
    if (!err.empty() or !warn.empty() or !ok)
        return {};

    return parseGltf(ctx, model);
}

}  // namespace btm

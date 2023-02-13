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

template<typename T>
auto digestMeshProp(tinygltf::Model const &model, int32_t accessorIdx)
{
    if (accessorIdx < 0)
        return ds::view((T *)nullptr, 0);

    auto const &accessor   = model.accessors[accessorIdx];
    auto const &bufferView = model.bufferViews[accessor.bufferView];
    auto const &buffer     = model.buffers[bufferView.buffer];
    auto const  offset     = bufferView.byteOffset;

    return ds::view(reinterpret_cast<T const *>(&buffer.data[offset]), accessor.count);
}

std::vector<Mesh> parseGltf(tinygltf::Model const &model)
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

            auto const &p = primitive;
            auto idx = [&p](const char *name) { return p.attributes.count(name) > 0 ? p.attributes.at(name) : -1; };

            // INDICES (must read as uint16_t)
            {
                auto const dataView = digestMeshProp<u16>(model, primitive.indices);
                ds::merge(outMesh.indices, dataView);
            }
            // POS
            {
                auto const dataView = digestMeshProp<glm::vec3>(model, idx("POSITION"));

                if (outMesh.vertices.empty())
                    outMesh.vertices.resize(dataView.size());

                for (size_t i = 0; i < dataView.size(); ++i)
                    outMesh.vertices[i].pos = dataView[i];
            }
            // UV0
            {
                auto const dataView = digestMeshProp<glm::vec2>(model, idx("TEXCOORD_Ø"));
                for (size_t i = 0; i < dataView.size(); ++i)
                    outMesh.vertices[i].uv0 = dataView[i];
            }
            // NORMAL
            {
                auto const dataView = digestMeshProp<glm::vec3>(model, idx("NORMAL"));
                for (size_t i = 0; i < dataView.size(); ++i)
                    outMesh.vertices[i].normal = dataView[i];
            }
            // TANGENT
            {
                auto const dataView = digestMeshProp<glm::vec4>(model, idx("TANGENT"));
                for (size_t i = 0; i < dataView.size(); ++i)
                    outMesh.vertices[i].tangent = dataView[i];
            }

            meshes.push_back(outMesh);
        }
    }

    return meshes;
}

std::vector<Mesh> parseGltf(std::string const &filepath)
{
    auto const bin   = btm::bin::read(filepath);
    auto const isBin = btm::bin::checkMagic(ds::view(bin, 4), { 'g', 'l', 'T', 'F' });

    if (bin.empty())
        return {};

    tinygltf::TinyGLTF ctx;
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

    return parseGltf(model);
}

std::vector<Mesh> parseGltf(std::span<u8> bin)
{
    tinygltf::TinyGLTF ctx;
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

    return parseGltf(model);
}

}  // namespace btm

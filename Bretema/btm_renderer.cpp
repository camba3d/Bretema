#include "btm_renderer.hpp"
#include "btm_utils.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION  // optional. disable exception handling.
#include "tiny_gltf.h"

namespace btm
{

//=========================================================
// Base Renderer
//=========================================================

BaseRenderer::BaseRenderer(sPtr<btm::Window> window)
{
    mWindow = window;
    BTM_ASSERT_X(mWindow->handle(), "Invalid window handle");
    BTM_ASSERT_X(w() > 0 && h() > 0, "Invalid viewport size");
}

//=========================================================
// GLTF Loader
//=========================================================

template<typename T>
auto gatherMeshData(tinygltf::Model const &model, i32 accessorIdx)
{
    if (accessorIdx < 0)
        return ds::make_view((T *)nullptr, 0);

    auto const &accessor   = model.accessors[accessorIdx];
    auto const &bufferView = model.bufferViews[accessor.bufferView];
    auto const &buffer     = model.buffers[bufferView.buffer];
    auto const  offset     = bufferView.byteOffset;

    return ds::make_view(reinterpret_cast<T const *>(&buffer.data[offset]), accessor.count);
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

            auto const &p   = primitive;
            auto        idx = [&p](const char *name) { return p.attributes.count(name) > 0 ? p.attributes.at(name) : -1; };

            // INDICES (must read as u16)
            {
                auto const dataView = gatherMeshData<u16>(model, primitive.indices);
                ds::merge<u16>(outMesh.indices, dataView);
            }
            // POS
            {
                auto const dataView = gatherMeshData<glm::vec3>(model, idx("POSITION"));

                if (outMesh.vertices.empty())
                    outMesh.vertices.resize(dataView.size());

                for (size_t i = 0; i < dataView.size(); ++i) outMesh.vertices[i].pos = dataView[i];
            }
            // UV0
            {
                auto const dataView = gatherMeshData<glm::vec2>(model, idx("TEXCOORD_Ø"));
                for (size_t i = 0; i < dataView.size(); ++i) outMesh.vertices[i].uv0 = dataView[i];
            }
            // NORMAL
            {
                auto const dataView = gatherMeshData<glm::vec3>(model, idx("NORMAL"));
                for (size_t i = 0; i < dataView.size(); ++i) outMesh.vertices[i].normal = dataView[i];
            }
            // TANGENT
            {
                auto const dataView = gatherMeshData<glm::vec4>(model, idx("TANGENT"));
                for (size_t i = 0; i < dataView.size(); ++i) outMesh.vertices[i].tangent = dataView[i];
            }

            meshes.push_back(outMesh);
        }
    }

    return meshes;
}

std::vector<Mesh> parseGltf(bool isBin, std::string const &filepath, ds::view<u8> bin)
{
    tinygltf::TinyGLTF ctx;
    tinygltf::Model    model;
    std::string        err, warn;

    bool const ok = isBin ? ctx.LoadBinaryFromMemory(&model, &err, &warn, bin.data(), (u32)bin.size())
                          : ctx.LoadASCIIFromFile(&model, &err, &warn, filepath);

    if (!err.empty())
        BTM_ERRF("Loading GLTF {}: {}", filepath, err);

    if (!warn.empty())
        BTM_WARNF("Loading GLTF {}: {}", filepath, warn);

    if (!ok and (err.empty() or warn.empty()))
        BTM_ERRF("Loading GLTF {}: Undefined error", filepath);

    if (!ok or !err.empty() or !warn.empty())
        return {};

    return parseGltf(model);
}

std::vector<Mesh> parseGltf(std::string const &filepath)
{
    auto const bin   = bin::read(filepath);
    bool const isBin = bin::checkMagic(ds::make_view(bin, 4), { 'g', 'l', 'T', 'F' });

    return parseGltf(isBin, filepath, ds::make_view(bin));
}

std::vector<Mesh> parseGltf(ds::view<u8> bin, std::string name)
{
    auto const isBin = bin::checkMagic(bin.subspan(0, 4), { 'g', 'l', 'T', 'F' });
    BTM_ASSERT(isBin);

    return parseGltf(true, name, bin);
}

}  // namespace btm

#include "btm_renderer.hpp"

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

// // Get glTF mesh attribute
void digestMeshAttribute(
  tinygltf::Model const     &model,
  tinygltf::Primitive const &primitive,
  std::string const         &name,
  std::vector<float>        &out,
  size_t                     components)
{
    int accessorIdx   = -1;
    int bufferViewIdx = -1;
    for (auto const &attribute : primitive.attributes)
    {
        if (attribute.first == name)
        {
            accessorIdx   = attribute.second;
            bufferViewIdx = model.accessors[accessorIdx].bufferView;
            break;
        }
    }

    if (accessorIdx < 0 or bufferViewIdx < 0)
    {
        // BTM_WARNF("Parsing {} : accessor or buffer not found", name);
        return;
    }

    tinygltf::Accessor const   &accessor = model.accessors[accessorIdx];
    tinygltf::BufferView const &view     = model.bufferViews[bufferViewIdx];
    tinygltf::Buffer const     &buffer   = model.buffers[view.buffer];
    float const *data = reinterpret_cast<float const *>(&buffer.data[view.byteOffset + accessor.byteOffset]);

    size_t const numVertices = accessor.count / components;
    // BTM_INFOF("Parsing {} : {} vertices (size {})", name, numVertices, components);

    for (size_t i = 0; i < numVertices; ++i)
    {
        size_t curr = components * i;
        out.push_back(data[curr + 0]);  // x
        out.push_back(data[curr + 1]);  // y
        if (components > 2)
            out.push_back(data[curr + 2]);  // z
        if (components > 3)
            out.push_back(data[curr + 3]);  // w
    }
}

// Load a glTF scene from a file path
std::vector<Mesh> parseGltf(std::string const &filepath)
{
    std::ifstream   file { filepath, std::ios::binary };
    auto            fileBegin = std::istreambuf_iterator<char>(file);
    auto            fileEnd   = std::istreambuf_iterator<char>();
    std::vector<u8> data { fileBegin, fileEnd };

    bool const isBinary = data[0] == 'g' and data[1] == 'l' and data[2] == 'T' and data[3] == 'F';

    BTM_INFOF("parseGltf => {} : {}", filepath, isBinary);

    if (data.empty())
    {
        BTM_ERRF("File '{}' empty or invalid", filepath);
        BTM_ASSERT(0);
        return {};
    }

    tinygltf::Model    model;
    tinygltf::TinyGLTF ctx;
    std::string        err, warn;

    bool const ok = !isBinary ? ctx.LoadASCIIFromFile(&model, &err, &warn, filepath)
                              : ctx.LoadBinaryFromMemory(&model, &err, &warn, data.data(), data.size());

    if (!err.empty())
        BTM_ERRF("Loading GLTF '{}' : {}", filepath, err);
    if (!warn.empty())
        BTM_WARNF("Loading GLTF '{}' : {}", filepath, warn);
    if (!ok)
        BTM_WARNF("Loading GLTF '{}' : Undefined error", filepath);
    if (!err.empty() or !warn.empty() or !ok)
        return {};

    std::vector<Mesh> meshes;

    for (const auto &mesh : model.meshes)
    {
        Mesh outMesh;
        outMesh.name = mesh.name;
        for (const auto &primitive : mesh.primitives)
        {
            if (primitive.mode != TINYGLTF_MODE_TRIANGLES)
                continue;

            digestMeshAttribute(model, primitive, "POSITION", outMesh.positions, 3);
            digestMeshAttribute(model, primitive, "TEXCOORD_Ø", outMesh.uvs0, 2);
            digestMeshAttribute(model, primitive, "NORMAL", outMesh.normals, 3);
            digestMeshAttribute(model, primitive, "TANGENT", outMesh.tangents, 4);
        }
        meshes.push_back(outMesh);
    }

    // BTM_INFOF("END parseGltf => {} : {}", filepath, meshes.size());
    BTM_INFOF("END parseGltf => {} : {}", filepath, meshes[0]);

    // if (meshes.size() > 0)
    //     for (auto const &v : meshes.at(0).normals)
    //         BTM_INFOF("aaaaaaaaaaaaaaaaaaaaa {}", v);

    return meshes;
}

}  // namespace btm
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

std::vector<Mesh> parseGltf(tinygltf::TinyGLTF const &ctx, tinygltf::Model const &model)
{
    std::vector<Mesh> meshes;

    for (auto const &mesh : model.meshes)
    {
        Mesh outMesh;
        outMesh.name = mesh.name;

        // ... Populate mesh
        //-------------------------------------
        for (const auto &primitive : mesh.primitives)
        {
            if (primitive.mode != TINYGLTF_MODE_TRIANGLES)
                continue;

            static std::array const sAttributes = {
                std::make_tuple(Mesh::Pos, 3, "POSITION"),    std::make_tuple(Mesh::UV0, 2, "TEXCOORD_Ø"),
                std::make_tuple(Mesh::Normal, 3, "NORMAL"),   std::make_tuple(Mesh::Tangent, 4, "TANGENT"),
                std::make_tuple(Mesh::Tangent, 4, "TANGENT"),
            };

            // INDICES

            const int                   indicesAccessorIndex   = primitive.indices;
            const tinygltf::Accessor   &indicesAccessor        = model.accessors[indicesAccessorIndex];
            const int                   indicesBufferViewIndex = indicesAccessor.bufferView;
            const tinygltf::BufferView &indicesBufferView      = model.bufferViews[indicesBufferViewIndex];
            const int                   indicesBufferIndex     = indicesBufferView.buffer;
            const tinygltf::Buffer     &indicesBuffer          = model.buffers[indicesBufferIndex];
            const void                 *indicesData            = &indicesBuffer.data[indicesBufferView.byteOffset];
            const uint16_t             *indicesArray           = static_cast<const uint16_t *>(indicesData);
            const int                   indicesCount           = indicesAccessor.count;

            //-----

            auto const  accessorIdx   = primitive.indices;
            auto const  bufferViewIdx = model.accessors[accessorIdx].bufferView;
            // assert(accessorIdx < 0 or bufferViewIdx < 0)
            auto const &accessor      = model.accessors[accessorIdx];
            auto const &bufferView    = model.bufferViews[bufferViewIdx];
            auto const  bufferIdx     = bufferView.buffer;
            auto const &buffer        = model.buffers[bufferView.buffer];
            auto const  offset        = bufferView.byteOffset + accessor.byteOffset;
            void const *raw           = &buffer.data[offset];
            auto const *data          = static_cast<uint32_t const *>(raw);
            //-----

            /* INDICES
            //=============================================
              // Get the accessor index for the indices of the primitive
              const int indicesAccessorIndex = primitive.indices;
              // Get the accessor for the indices
              const tinygltf::Accessor &indicesAccessor = model.accessors[indicesAccessorIndex];
              // Get the buffer view index for the indices
              const int indicesBufferViewIndex = indicesAccessor.bufferView;
              // Get the buffer view for the indices
              const tinygltf::BufferView &indicesBufferView = model.bufferViews[indicesBufferViewIndex];
              // Get the buffer index for the indices
              const int indicesBufferIndex = indicesBufferView.buffer;
              // Get the buffer for the indices
              const tinygltf::Buffer &indicesBuffer = model.buffers[indicesBufferIndex];
              // Get the data pointer for the indices
              const void *indicesData = &indicesBuffer.data[indicesBufferView.byteOffset];
              // Cast the data pointer to the desired data type (for example, uint16_t)
              const uint16_t *indicesArray = static_cast<const uint16_t *>(indicesData);
              // The number of elements in the indices array is determined by the count property of the accessor
              const int indicesCount = indicesAccessor.count;
              // Do something with the indices array, for example, print it
              for (int i = 0; i < indicesCount; i++) {
                std::cout << indicesArray[i] << " ";
              }
            */

            for (auto const &[type, components, name] : sAttributes)
            {
                // ... Gather indices
                //-------------------------------------
                int accessorIdx   = -1;
                int bufferViewIdx = -1;

                //@dani : Segment the process in helper functions, it will help to simplify Indicies read also.
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
                    continue;
                }

                BTM_INFOF("iiiiiiiiindex => {}/{}", accessorIdx, bufferViewIdx);

                // ... Gather data
                //-------------------------------------
                tinygltf::Accessor const   &accessor = model.accessors[accessorIdx];
                tinygltf::BufferView const &view     = model.bufferViews[bufferViewIdx];
                tinygltf::Buffer const     &buffer   = model.buffers[view.buffer];
                int32_t const               offset   = view.byteOffset + accessor.byteOffset;
                float const                *data     = reinterpret_cast<float const *>(&buffer.data[offset]);

                // BTM_INFOF("Parsing {} : {} vertices (size {})", name, numVertices, components);

                // ... Store data in atrributes
                //-------------------------------------
                size_t const numVertices = accessor.count / components;
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

    BTM_INFOF("END ParseGltf => {}", meshes[0]);

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

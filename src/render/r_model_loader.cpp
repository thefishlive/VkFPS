/******************************************************************************
* Copyright 2017 James Fitzpatrick <james_fitzpatrick@outlook.com>           *
*                                                                            *
* Permission is hereby granted, free of charge, to any person obtaining a    *
* copy of this software and associated documentation files (the "Software"), *
* to deal in the Software without restriction, including without limitation  *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,   *
* and/or sell copies of the Software, and to permit persons to whom the      *
* Software is furnished to do so, subject to the following conditions:       *
*                                                                            *
* The above copyright notice and this permission notice shall be included in *
* all copies or substantial portions of the Software.                        *
*                                                                            *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    *
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        *
* DEALINGS IN THE SOFTWARE.                                                  *
******************************************************************************/
#include "r_model_loader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "u_io.h"
#include "u_debug.h"
#include "u_defines.h"

ModelLoader::ModelLoader(std::shared_ptr<GraphicsDevice> &device, std::shared_ptr<GraphicsDevmem> &devmem, std::shared_ptr<Renderer> &renderer)
    : device(device), devmem(devmem), renderer(renderer), image_loader(std::make_unique<RenderImageLoader>(device, devmem))
{
    this->create_dummy_texture_sampler();
}

ModelLoader::~ModelLoader()
{
}

glm::vec4 ModelLoader::get_vertex(const tinyobj::attrib_t& attrib, int vertex_index)
{
    if (vertex_index == -1)
        return { 0, 0, 0, 0 };

    return { attrib.vertices[3 * vertex_index], attrib.vertices[3 * vertex_index + 1], attrib.vertices[3 * vertex_index + 2], 1.0f };
}

glm::vec4 ModelLoader::get_normal(const tinyobj::attrib_t& attrib, int index)
{
    if (index == -1)
        return { 0, 0, 0, 0 };

    return { attrib.normals[3 * index], attrib.normals[3 * index + 1], attrib.normals[3 * index + 2], 1.0f };
}

glm::vec2 ModelLoader::get_uv(const tinyobj::attrib_t& attrib, int index)
{
    if (index == -1)
        return { 0, 0 };

    return { attrib.texcoords[2 * index], 1 - attrib.texcoords[2 * index + 1] };
}

glm::vec4 ModelLoader::to_vec4(float f[3])
{
    return glm::vec4(f[0], f[1], f[2], f[3]);
}

std::unique_ptr<Model> ModelLoader::load_model(std::string path)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> obj_materials;
    std::string err;

    LOG_INFO("Loading model %s", path.c_str());

    if (!tinyobj::LoadObj(&attrib, &shapes, &obj_materials, &err, FILENAME_TO_PATH(path).c_str(), FILENAME_TO_PATH(std::string("models")).c_str()))
    {
        throw std::runtime_error(err);
    }

    if (err.length() > 0)
    {
        LOG_WARN("TinyObjLoading Output:\n%s", err.c_str());
    }

    std::vector<Vertex> verticies;
    std::vector<std::vector<uint32_t>> indicies;
    std::vector<std::unique_ptr<Material>> materials;

    materials.push_back(
        std::move(
            std::make_unique<Material>(
                device,
                renderer,
                glm::vec4(1, 1, 1, 1),
                glm::vec4(1, 1, 1, 1),
                glm::vec4(1, 1, 1, 1),
                1.0f,
                dummy_image,
                dummy_image,
                dummy_image
                )
        )
    );

    for (auto material : obj_materials)
    {
        std::shared_ptr<GraphicsDevmemImage> image;

        if (material.diffuse_texname.length() > 0)
        {
            image = image_loader->load_image(FILENAME_TO_PATH(material.diffuse_texname).c_str());
        }
        else
        {
            image = dummy_image;
        }

        materials.push_back(
            std::move(
                std::make_unique<Material>(
                    device, 
                    renderer, 
                    to_vec4(material.ambient), 
                    to_vec4(material.diffuse), 
                    to_vec4(material.specular), 
                    material.dissolve,
                    dummy_image,
                    image,
                    dummy_image
                )
            )
        );
    }

    indicies.resize(obj_materials.size() + 1);

    int v = 0, i = 0, m = 0;

    for (auto shape : shapes)
    {
        for (auto num_vertices : shape.mesh.num_face_vertices)
        {
            for (int j = 0; j < num_vertices; i++, j++)
            {
                auto index = shape.mesh.indices[i];

                verticies.push_back(
                    Vertex(
                        get_vertex(attrib, index.vertex_index),
                        get_normal(attrib, index.normal_index),
                        { 1.0f, 1.0f, 1.0f, 1.0f },
                        get_uv(attrib, index.texcoord_index)
                    )
                );

                indicies[shape.mesh.material_ids[m] + 1].push_back(v++);
            }

            m++;
        }
    }

    return std::make_unique<Model>(device, devmem, renderer, verticies, materials, indicies);
}

std::string ModelLoader::get_library_path(const std::string& library, const std::string& file)
{
    return file.substr(0, file.find_last_of('\\') + 1) + library;
}

void ModelLoader::create_dummy_texture_sampler()
{
    VkDeviceSize image_size = 1 * 1 * 4;

    LOG_INFO("Allocating dummy 1x1 white texture");

    vk::BufferCreateInfo staging_buffer_create_info(
        vk::BufferCreateFlags(),
        image_size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::SharingMode::eExclusive,
        0,
        nullptr
    );

    VmaAllocationCreateInfo staging_buffer_alloc_info{};
    staging_buffer_alloc_info.flags = 0;
    staging_buffer_alloc_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    staging_buffer_alloc_info.pUserData = STRING_TO_DATA("DummyImageStagingBuffer");

    std::unique_ptr<GraphicsDevmemBuffer> staging_buffer = devmem->create_buffer(staging_buffer_create_info, staging_buffer_alloc_info);

    uint32_t *data;
    staging_buffer->map_memory((void **) &data);
    *data = UINT32_MAX; /* WHITE */
    staging_buffer->unmap_memory();

    vk::ImageCreateInfo image_create_info(
        vk::ImageCreateFlags(),
        vk::ImageType::e2D,
        vk::Format::eR8G8B8A8Uint,
        vk::Extent3D(1, 1, 1),
        1,
        1,
        vk::SampleCountFlagBits::e1,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        vk::SharingMode::eExclusive,
        0,
        nullptr,
        vk::ImageLayout::eUndefined
    );

    VmaAllocationCreateInfo image_alloc_info{};
    image_alloc_info.flags = VMA_ALLOCATION_CREATE_EXT_DONT_CREATE_STAGING_BUFFER;
    image_alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    image_alloc_info.pUserData = STRING_TO_DATA("DummyImage");

    std::unique_ptr<GraphicsDevmemImage> image = devmem->create_image(image_create_info, image_alloc_info);
    image->transition_layout(vk::ImageLayout::eTransferDstOptimal);

    vk::BufferImageCopy region(
        0, 0,
        0,
        vk::ImageSubresourceLayers(
            vk::ImageAspectFlagBits::eColor,
            0,
            0,
            1
        ),
        vk::Offset3D(
            0,
            0,
            0
        ),
        vk::Extent3D(
            1,
            1,
            1
        )
    );

    auto batch = device->transfer_context->start_batch(GraphicsTransferHardwareDest::eGraphics);
    batch->blit_buffer_to_image(staging_buffer->buffer, image->image, vk::ImageLayout::eTransferDstOptimal, region);
    device->transfer_context->end_batch(std::move(batch), false);

    device->device.waitIdle();

    dummy_image = std::move(image);
}

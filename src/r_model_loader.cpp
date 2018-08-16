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

ModelLoader::ModelLoader(std::shared_ptr<GraphicsDevice> &device, std::shared_ptr<GraphicsDevmem> &devmem, std::shared_ptr<Renderer> &renderer)
    : device(device), devmem(devmem), renderer(renderer)
{
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

    return { attrib.texcoords[3 * index], attrib.texcoords[3 * index + 1] };
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

    if (!tinyobj::LoadObj(&attrib, &shapes, &obj_materials, &err, FILENAME_TO_PATH(path).c_str(), FILENAME_TO_PATH(std::string("models")).c_str()))
    {
        throw std::runtime_error(err);
    }

    std::vector<Vertex> verticies;
    std::vector<std::vector<uint32_t>> indicies;
    std::vector<std::unique_ptr<Material>> materials;

    for (auto material : obj_materials)
    {
        materials.push_back(
            std::move(
                std::make_unique<Material>(
                    device, 
                    renderer, 
                    to_vec4(material.ambient), 
                    to_vec4(material.diffuse), 
                    to_vec4(material.specular), 
                    material.dissolve
                )
            )
        );
    }

    indicies.resize(obj_materials.size());

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

                indicies[shape.mesh.material_ids[m]].push_back(v++);
            }

            m++;
        }
    }

    return std::make_unique<Model>(device, devmem, renderer, verticies, materials, indicies);
}

std::string ModelLoader::get_library_path(const std::string& library, const std::string& file)
{
    return file.substr(0, file.find_last_of('/') + 1) + library;
}

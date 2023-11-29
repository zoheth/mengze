#pragma once
#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION 
#include "tiny_obj_loader.h"

#include "rendering/camera.h"

std::string base_dir = "scenes\\bathroom\\";
std::string input_file = "scenes\\bathroom\\bathroom.obj";
tinyobj::attrib_t attrib;
std::vector<tinyobj::shape_t> shapes;
std::vector<tinyobj::material_t> materials;

void load();

float linearize_depth(float depth, float near, float far)
{
    float linearized_depth = near * far / (far - depth * (far - near));
    return (linearized_depth-near)/(far - near);
}

class ZbufferRenderer : public mengze::Renderer
{
public:
	explicit ZbufferRenderer(mengze::Camera& camera) : camera_(camera)
	{
		load();
	}

    void on_update(float ts) override
    {
	    camera_.on_update(ts);
        if(depth_buffer_ && get_width() * get_height()>0)
			std::fill_n(depth_buffer_, get_width() * get_height(), std::numeric_limits<float>::max());
    }

    void on_resize(uint32_t width, uint32_t height) override
    {
        camera_.on_resize(width, height);
        mengze::Renderer::on_resize(width, height);

        delete[] depth_buffer_;
        depth_buffer_ = new float[width * height];
        std::fill_n(depth_buffer_, width * height, std::numeric_limits<float>::max());
    }

    void render() override
    {
        const auto& projection_matrix = camera_.get_projection_matrix();
        const auto& view_matrix = camera_.get_view_matrix();

        for (const auto& shape : shapes) {
            size_t index_offset = 0;
            for (const auto fv : shape.mesh.num_face_vertices) {
                for (size_t v = 0; v < fv; v++) {

                    tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
                    glm::vec4 vertex_position = glm::vec4(attrib.vertices[3 * idx.vertex_index + 0],
                        attrib.vertices[3 * idx.vertex_index + 1],
                        attrib.vertices[3 * idx.vertex_index + 2],
                        1.0f);


                    glm::vec4 clip_space_pos = projection_matrix * view_matrix * vertex_position;


                    glm::vec3 ndc_pos = glm::vec3(clip_space_pos) / clip_space_pos.w;


                    uint32_t screen_x = static_cast<uint32_t>((ndc_pos.x + 1.0f) * 0.5f * get_width());
                    uint32_t screen_y = static_cast<uint32_t>((1.0f - ndc_pos.y) * 0.5f * get_height());


                    if (ndc_pos.x >= -1.0f && ndc_pos.x <= 1.0f &&
                        ndc_pos.y >= -1.0f && ndc_pos.y <= 1.0f &&
                        ndc_pos.z >=  0.0f && ndc_pos.z <= 1.0f) {

                        uint32_t screen_x = static_cast<uint32_t>((ndc_pos.x + 1.0f) * 0.5f * get_width());
                        uint32_t screen_y = static_cast<uint32_t>((1.0f - ndc_pos.y) * 0.5f * get_height());

                        float z_value = ndc_pos.z;
                        float linearized_z_value = linearize_depth(ndc_pos.z, camera_.get_near(), camera_.get_far());

                        if (z_value < depth_buffer_[screen_y * get_width() + screen_x])
                        {
                            set_pixel(screen_x, screen_y, glm::vec3(linearized_z_value));
                            depth_buffer_[screen_y * get_width() + screen_x] = z_value;
                        }
                    }
                }
                index_offset += fv;
            }
        }

    }
private:
    mengze::Camera& camera_;
    float* depth_buffer_ {nullptr};
};

void load()
{
    std::string warn;
    std::string err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, input_file.c_str(), base_dir.c_str());

    if (!warn.empty()) {
        std::cout << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << err << std::endl;
    }

    if (!ret) {
        exit(1);
    }

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                tinyobj::real_t vx = attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 0];
                tinyobj::real_t vy = attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 1];
                tinyobj::real_t vz = attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 2];

                // Check if `normal_index` is zero or positive. negative = no normal data
                if (idx.normal_index >= 0) {
                    tinyobj::real_t nx = attrib.normals[3 * static_cast<size_t>(idx.normal_index) + 0];
                    tinyobj::real_t ny = attrib.normals[3 * static_cast<size_t>(idx.normal_index) + 1];
                    tinyobj::real_t nz = attrib.normals[3 * static_cast<size_t>(idx.normal_index) + 2];
                }

                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0) {
                    tinyobj::real_t tx = attrib.texcoords[2 * static_cast<size_t>(idx.texcoord_index) + 0];
                    tinyobj::real_t ty = attrib.texcoords[2 * static_cast<size_t>(idx.texcoord_index) + 1];
                }
                // Optional: vertex colors
                // tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
                // tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
                // tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];
            }
            index_offset += fv;

            // per-face material
            shapes[s].mesh.material_ids[f];
        }
    }
}


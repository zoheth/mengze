#pragma once
#include "rendering/camera.h"
#include "rendering/renderer.h"

#include <glm/glm.hpp>
#include "glm/ext/matrix_clip_space.hpp"

#include "edge_table.h"
#include "geometry.h"
#include "core/application.h"


namespace mengze
{
	glm::vec3 simple_shading(const glm::vec3& normal);

	class Rasterizer : public Renderer
	{
	public:
		explicit Rasterizer(Camera& camera, Geometry& geometry);


		void on_update(float ts) override;

		void on_resize(uint32_t width, uint32_t height) override;


		void render() override
		{
			clear(glm::vec3(0.1f));
			for (uint32_t i = 0; i < screen_vertices_.size(); ++i)
			{
				screen_vertices_[i] = to_screen_space(geometry_.get_vertices()[i]);
			}
			render_triangle();

		}

		virtual  void render_triangle() = 0;

		Triangle get_screen_triangle(uint32_t index) const;

		Triangle get_world_triangle(uint32_t index) const;

		bool check_screen_triangle(const Triangle& triangle) const;

	protected:
		uint32_t num_triangles_;
		Camera& camera_;
		Geometry& geometry_;
		std::vector<glm::vec3> screen_vertices_;
		float* depth_buffer_{ nullptr };

	private:
		glm::vec3 to_screen_space(const glm::vec3& vertex) const;

	private:
	};
}

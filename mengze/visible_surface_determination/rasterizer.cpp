#include "rasterizer.h"

#include "core/timer.h"

namespace 
{
	mengze::Timer timer;

	bool is_valid_number(float value) {
		return !(std::isnan(value) || std::isinf(value));
	}

	bool check_triangle_vertices(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) {
		if (is_valid_number(v0.x) && is_valid_number(v0.y) && is_valid_number(v0.z) &&
			is_valid_number(v1.x) && is_valid_number(v1.y) && is_valid_number(v1.z) &&
			is_valid_number(v2.x) && is_valid_number(v2.y) && is_valid_number(v2.z)) {
			return true;
		}

		return false;

	}
}

namespace mengze
{
	glm::vec3 simple_shading(const glm::vec3& normal) {

		glm::vec3 ambient(0.1f);
		glm::vec3 light_position(1.0f);

		glm::vec3 light_dir = glm::normalize(light_position - glm::vec3(0.0f));
		float diff1 = std::max(glm::dot(normal, light_dir), 0.0f);
		glm::vec3 diffuse1(0.5f * diff1);

		light_dir = glm::normalize(glm::vec3(-1.0f, 1.0f, -1.0f));
		float diff2 = std::max(glm::dot(normal, light_dir), 0.0f);
		glm::vec3 diffuse2(0.5f * diff1);

		/*glm::vec3 halfDir = glm::normalize(light_dir + view_dir);
		float spec = pow(std::max(glm::dot(normal, halfDir), 0.0f), 32.0f);
		glm::vec3 specular(1.0f * spec);*/

		glm::vec3 color = ambient + diffuse1 + diffuse2;
		return color;
	}

	Rasterizer::Rasterizer(Camera& camera, Geometry& geometry): camera_(camera), geometry_(geometry)
	{
		screen_vertices_.resize(geometry_.get_vertices().size());
		num_triangles_ = geometry_.get_num_triangles();
	}

	void Rasterizer::on_update(float ts)
	{

		camera_.on_update(ts);
		reset_depth_buffer();

	}

	void Rasterizer::on_resize(uint32_t width, uint32_t height)
	{
		camera_.on_resize(width, height);
		mengze::Renderer::on_resize(width, height);

		resize_depth_buffer(width, height);
	}

	void Rasterizer::render()
	{
		timer.reset();
		clear(glm::vec3(0.1f));
		for (uint32_t i = 0; i < screen_vertices_.size(); ++i)
		{
			screen_vertices_[i] = to_screen_space(geometry_.get_vertices()[i]);
		}
		vertex_transform_time_ = timer.elapsed();
		timer.reset();
		render_triangle();
		rasterization_time_ = timer.elapsed();

	}

	void Rasterizer::resize_depth_buffer(uint32_t width, uint32_t height)
	{
		delete[] depth_buffer_;
		depth_buffer_ = new float[width * height];
		reset_depth_buffer();
	}

	void Rasterizer::reset_depth_buffer()
	{
		if (depth_buffer_ && get_width() * get_height() > 0)
			std::fill_n(depth_buffer_, get_width() * get_height(), std::numeric_limits<float>::max());
	}

	Triangle Rasterizer::get_screen_triangle(uint32_t index) const
	{
		return geometry_.get_triangle(index, &screen_vertices_);
	}

	Triangle Rasterizer::get_world_triangle(uint32_t index) const
	{
		return geometry_.get_triangle(index);
	}

	bool Rasterizer::check_screen_triangle(const Triangle& triangle) const
	{
		const glm::vec3& v0 = triangle.vertices[0];
		const glm::vec3& v1 = triangle.vertices[1];
		const glm::vec3& v2 = triangle.vertices[2];
		if (v0.z < 0.f || v0.z>1.f || v1.z < 0.f || v1.z>1.f || v2.z < 0.f || v2.z>1.f)
			return false;
		if (!check_triangle_vertices(v0, v1, v2))
			return false;
		float min_x = std::min({ v0.x, v1.x, v2.x });
		float max_x = std::max({ v0.x, v1.x, v2.x });
		float min_y = std::min({ v0.y, v1.y, v2.y });
		float max_y = std::max({ v0.y, v1.y, v2.y });

		if (min_x < 0 || min_y < 0 || max_x >= get_width() || max_y >= get_height())
			return false;
		return true;
	}

	glm::vec3 Rasterizer::to_screen_space(const glm::vec3& vertex) const
	{
		glm::vec4 v = camera_.get_view_matrix() * glm::vec4(vertex, 1.0f);
		v = camera_.get_projection_matrix() * v;
		v /= v.w;
		v.x = (v.x + 1.0f) * 0.5f * static_cast<float>(get_width());
		v.y = (1.0f - v.y) * 0.5f * static_cast<float>(get_height());
		return v;
	}
	
}

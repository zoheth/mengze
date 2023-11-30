#pragma once
#include <fstream>
#include <sstream>
#include <vector>
#include <array>
#include <functional>

#include <glm/glm.hpp>

#include "core/logging.h"


inline void transform_vertices_range(
	std::vector<glm::vec3>& vertices,
	const std::function<glm::vec3(const glm::vec3&)>& func,
	int start,
	int end)
{
	for (int i = start; i < end; ++i) {
		vertices[i] = func(vertices[i]);
	}
}

namespace mengze
{
	struct Triangle
	{
		Triangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2)
			: vertices{ v0, v1, v2 } {}

		std::array<glm::vec3, 3> vertices;

	};

	class TriangleIterator
	{
	public:
		TriangleIterator(const std::vector<uint32_t>& indices, const std::vector<glm::vec3>& vertices, size_t position);

		bool operator!=(const TriangleIterator& other) const;

		TriangleIterator& operator++();

		Triangle operator*() const;

	private:
		const std::vector<uint32_t>& indices_;
		const std::vector<glm::vec3>& vertices_;
		size_t position_;
	};


	class Geometry
	{
	public:
		Geometry(const std::string& path);

		std::vector<glm::vec3>& get_vertices() { return vertices_; }

		std::vector<uint32_t>& get_indices() { return indices_; }

		uint32_t get_num_triangles() const { return num_triangles_; }

		Triangle get_triangle(uint32_t index, const std::vector<glm::vec3>* p_vertices = nullptr) const;

		TriangleIterator triangles_begin(const std::vector<glm::vec3>* p_vertices = nullptr) const;

		TriangleIterator triangles_end(const std::vector<glm::vec3>* p_vertices = nullptr) const;

	private:
		void parse_obj(const std::string& path);

	private:
		std::vector<glm::vec3> vertices_;
		std::vector<uint32_t> indices_;
		uint32_t num_triangles_;
	};

}

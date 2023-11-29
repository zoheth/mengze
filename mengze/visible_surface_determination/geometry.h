#pragma once
#include <fstream>
#include <sstream>
#include <vector>
#include <functional>
#include <future>

#include <ctpl.h>
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
	class Triangle
	{
	public:
		std::vector<glm::vec3> vertices;
		void add_vertex(const glm::vec3& vertex)
		{
			vertices.push_back(vertex);
			if (vertices.size() > 3)
			{
				LOGE("Only support triangle");
			}
		}
	};

	class Geometry
	{
	public:
		Geometry(const std::string& path);

		std::vector<glm::vec3>& get_vertices() { return vertices_; }

		std::vector<uint32_t>& get_indices() { return indices_; }

		void transform_vertices_multithreaded(
			std::function<glm::vec3(const glm::vec3&)> func,
			int num_threads);
	private:
		void parse_obj(const std::string& path);

	private:
		std::vector<glm::vec3> vertices_;
		std::vector<uint32_t> indices_;
	};

}

#include "geometry.h"

namespace mengze
{
	Geometry::Geometry(const std::string& path)
	{
		parse_obj(path);
	}

	void Geometry::parse_obj(const std::string& path)
	{
		std::ifstream file(path);
		std::string line;

		if (!file.is_open())
		{
			LOGE("Cannot open file: %s", path.c_str());
			return;
		}

		while (std::getline(file, line))
		{
			std::istringstream iss(line);
			std::string prefix;
			iss >> prefix;

			if (prefix == "v")
			{
				glm::vec3 vertex;
				iss >> vertex.x >> vertex.y >> vertex.z;
				vertices_.push_back(vertex);
			}
			else if (prefix == "f")
			{
				Triangle triangle;
				std::string vertex;
				while (iss >> vertex)
				{
					std::istringstream vertex_stream(vertex);
					int vertex_index;
					vertex_stream >> vertex_index;
					triangle.add_vertex(vertices_[vertex_index - 1]);
				}
				triangles_.push_back(triangle);
			}
		}

	}

	void Geometry::transform_vertices_multithreaded(std::function<glm::vec3(const glm::vec3&)> func,
		int num_threads)
	{
		ctpl::thread_pool pool(num_threads);
		const int num_vertices = vertices_.size();
		const int chunk_size = num_vertices / num_threads;

		std::vector<std::future<void>> futures;
		for (int i = 0; i < num_threads; ++i) {
			int start = i * chunk_size;
			int end = (i == num_threads - 1) ? num_vertices : (i + 1) * chunk_size;
			futures.push_back(pool.push([this, func, start, end](int) {
				transform_vertices_range(this->vertices_, func, start, end);
			}));
		}

		for (auto& future : futures) {
			future.get();
		}
	}
}

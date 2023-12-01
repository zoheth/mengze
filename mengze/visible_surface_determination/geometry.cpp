#include "geometry.h"

namespace mengze
{

	Geometry::Geometry(const std::string& path)
	{
		parse_obj(path);
	}

	Triangle Geometry::get_triangle(uint32_t index, const std::vector<glm::vec3>* p_vertices) const
	{
		assert(index < num_triangles_);
		if (p_vertices)
		{
			assert(vertices_.size() == p_vertices->size());
			return { p_vertices->at(indices_[index * 3]), p_vertices->at(indices_[index * 3 + 1]), p_vertices->at(indices_[index * 3 + 2]) };
		}
		return { vertices_[indices_[index * 3]],
						vertices_[indices_[index * 3 + 1]],
						vertices_[indices_[index * 3 + 2]] };
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
				std::string vertex;
				uint32_t i = 0;
				while (iss >> vertex)
				{
					if (i++ > 3)
					{
						LOGE("Only support triangle");
					}
					std::istringstream vertex_stream(vertex);
					uint32_t vertex_index;
					vertex_stream >> vertex_index;
					while (vertex_stream.peek() == '/')
					{
						vertex_stream.ignore();
						if (vertex_stream.peek() == '/')
						{
							vertex_stream.ignore();
						}
					}
					indices_.push_back(vertex_index-1);
				}
			}
		}
		num_triangles_ = indices_.size() / 3;

	}
}

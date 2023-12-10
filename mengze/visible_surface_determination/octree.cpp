#include "octree.h"

namespace mengze
{
	bool BoundingBox::contains(const glm::vec3& point) const
	{
		return (point.x >= min.x && point.x <= max.x) &&
			(point.y >= min.y && point.y <= max.y) &&
			(point.z >= min.z && point.z <= max.z);
	}

	bool BoundingBox::intersects(const BoundingBox& other) const
	{
		return (min.x <= other.max.x && max.x >= other.min.x) &&
			(min.y <= other.max.y && max.y >= other.min.y) &&
			(min.z <= other.max.z && max.z >= other.min.z);
	}

	void OctreeNode::insert(uint32_t triangle_index, const std::vector<glm::vec3>& vertices,
		const std::vector<uint32_t>& indices)
	{
		BoundingBox triangle_bounds = compute_triangle_bounding_box(triangle_index, vertices, indices);

		if (children[0] == nullptr) {
			triangle_indices.push_back(triangle_index);
			if (triangle_indices.size() > MAX_TRIANGLES_PER_NODE && depth < MAX_DEPTH) {
				subdivide(vertices, indices);
			}
		}
		else {
			bool fits_in_children = false;
			for (auto& child : children) {
				if (child->bounds.intersects(triangle_bounds)) {
					child->insert(triangle_index, vertices, indices);
					fits_in_children = true;
				}
			}

			if (!fits_in_children) {
				triangle_indices.push_back(triangle_index);
			}
		}
	}

	void OctreeNode::subdivide(const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices)
	{
		glm::vec3 center = (bounds.min + bounds.max) * 0.5f;
		glm::vec3 extent = (bounds.max - bounds.min) * 0.5f;

		for (int i = 0; i < 8; ++i) {
			glm::vec3 newMin = bounds.min;
			glm::vec3 newMax = center;

			if (i & 1) newMax.x = bounds.max.x; else newMin.x = center.x;
			if (i & 2) newMax.y = bounds.max.y; else newMin.y = center.y;
			if (i & 4) newMax.z = bounds.max.z; else newMin.z = center.z;

			children[i] = std::make_unique<OctreeNode>(BoundingBox{ newMin, newMax });
			children[i]->depth = depth + 1;
		}

		std::vector<uint32_t> triangles_to_keep;

		for (uint32_t triangle_index : triangle_indices) {
			BoundingBox triangleBounds = compute_triangle_bounding_box(triangle_index, vertices, indices);
			bool fitsInChildren = false;

			for (auto& child : children) {
				if (child->bounds.intersects(triangleBounds)) {
					child->triangle_indices.push_back(triangle_index);
					fitsInChildren = true;
				}
			}

			if (!fitsInChildren) {
				triangles_to_keep.push_back(triangle_index);
			}
		}
            
		triangle_indices = std::move(triangles_to_keep);
	}

	BoundingBox OctreeNode::compute_triangle_bounding_box(uint32_t triangle_index,
		const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices) const
	{
		BoundingBox bbox;
		bbox.min = bbox.max = vertices[indices[triangle_index * 3]];
		for (int i = 1; i < 3; ++i) {
			glm::vec3 vertex = vertices[indices[triangle_index * 3 + i]];
			bbox.min = glm::min(bbox.min, vertex);
			bbox.max = glm::max(bbox.max, vertex);
		}
		return bbox;
	}

	Octree::Octree(const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices)
	{
		if (vertices.empty()) {
			throw std::runtime_error("Vertices array is empty.");
		}
		glm::vec3 min_point = vertices[0];
		glm::vec3 max_point = vertices[0];
            
		for (const auto& vertex : vertices) {
			min_point = glm::min(min_point, vertex);
			max_point = glm::max(max_point, vertex);
		}
            
		root_ = std::make_unique<OctreeNode>(BoundingBox{ min_point, max_point });
            
		insert_triangles(vertices, indices);
	}

	void Octree::insert_triangles(const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices) const
	{
		if (indices.size() % 3 != 0) {
			throw std::runtime_error("Indices array size is not a multiple of 3.");
		}
		for (uint32_t i = 0; i < indices.size(); i += 3) {
			root_->insert(i / 3, vertices, indices);
		}
	}
}

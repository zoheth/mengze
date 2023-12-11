#pragma once

#include <vector>
#include <array>
#include <memory>
#include <stdexcept>

#include <glm/glm.hpp>


namespace mengze
{
    struct BoundingBox {
        glm::vec3 min;
        glm::vec3 max;

        bool contains(const glm::vec3& point) const;

        bool intersects(const BoundingBox& other) const;
    };

    struct OctreeNode {
        BoundingBox bounds;
        std::vector<uint32_t> triangle_indices;
        std::array<std::unique_ptr<OctreeNode>, 8> children;

        explicit OctreeNode(const BoundingBox& b) : bounds(b) {}

        void insert(uint32_t triangle_index, const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices);

        void subdivide(const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices);

        BoundingBox compute_triangle_bounding_box(uint32_t triangle_index, const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices) const;

    private:
        static constexpr uint32_t MAX_TRIANGLES_PER_NODE = 5000;
        static constexpr uint32_t MAX_DEPTH = 3;
        uint32_t depth = 0;
    };

    class Octree {
    public:
        std::unique_ptr<OctreeNode> root_;

        Octree(const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices);

        void insert_triangles(const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices) const;
    };
}
#pragma once
#include <memory>
#include <vector>

#include <assimp/scene.h>

#include "ray_tracing/hittable.h"
#include "ray_tracing/camera.h"

namespace mengze::rt
{

class HittableList : public Hittable
{
  public:
	void add(const std::shared_ptr<Hittable> &object);

	bool hit(const Ray &r, Interval ray_t, HitRecord &rec) const override;

	float pdf_value(const glm::vec3 &origin, const glm::vec3 &direction) const override;

	glm::vec3 random(const glm::vec3 &origin) const override;

	const std::vector<std::shared_ptr<Hittable>> &objects() const;

	Aabb bounding_box() const override;

	bool empty() const;

  private:
	std::vector<std::shared_ptr<Hittable>> objects_;
	Aabb box_;
};

class Scene
{
  public:
	void parse_3d_model(const std::string &file_path);

	void parse_xml(const std::string &file_path);

	void add(const std::shared_ptr<Hittable> &object);

	void add_light(const std::shared_ptr<Hittable> &light);

	std::shared_ptr<Camera> camera() const
	{
		return camera_;
	}

	HittableList &world()
	{
		return world_;
	}
	HittableList &lights()
	{
		return lights_;
	}

private:
	void process_node(const aiNode *node, const aiScene *scene);
	void process_mesh(const aiMesh *mesh, const aiScene *scene);
	std::shared_ptr<Material> process_material(const aiMaterial *ai_material);

  private:
	HittableList world_;
	HittableList lights_;

	std::unordered_map<std::string, glm::vec3> lights_radiance_;

	std::shared_ptr<Camera> camera_;

	MaterialLibrary material_library_;
};
}        // namespace mengze::rt

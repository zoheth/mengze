#include "ray_tracing/scene.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "core/logging.h"
#include "ray_tracing/bvh.h"
#include "ray_tracing/triangle.h"

namespace mengze::rt
{
void HittableList::add(const std::shared_ptr<Hittable> &object)
{
	objects_.push_back(object);
}

bool HittableList::hit(const Ray &r, Interval ray_t, HitRecord &rec) const
{
	HitRecord temp_rec;
	bool      hit_anything   = false;
	auto      closest_so_far = ray_t.max();

	for (const auto &object : objects_)
	{
		if (object->hit(r, Interval{ray_t.min(), closest_so_far}, temp_rec))
		{
			hit_anything   = true;
			closest_so_far = temp_rec.t;
			rec            = temp_rec;
		}
	}

	return hit_anything;
}

float HittableList::pdf_value(const glm::vec3 &origin, const glm::vec3 &direction) const
{
	auto weight = 1.0f / objects_.size();
	auto sum    = 0.0f;

	for (const auto &object : objects_)
	{
		sum += weight * object->pdf_value(origin, direction);
	}
	return sum;
}

glm::vec3 HittableList::random(const glm::vec3 &origin) const
{
	auto index = static_cast<int>(random_float() * objects_.size());
	return objects_[index]->random(origin);
}

void Scene::parse_3d_model(const std::string &file_path)
{
	Assimp::Importer importer;
	const aiScene   *ai_scene = importer.ReadFile(file_path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!ai_scene || ai_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !ai_scene->mRootNode)
	{
		LOGE("Assimp error: {}", importer.GetErrorString())
		return;
	}
	process_node(ai_scene->mRootNode, ai_scene);
}

void Scene::add(const std::shared_ptr<Hittable> &object)
{
	world_.add(object);
}

void Scene::add_light(const std::shared_ptr<Hittable> &light)
{
	lights_.add(light);
}

void Scene::process_node(const aiNode *node, const aiScene *scene)
{
	for (size_t i = 0; i < node->mNumMeshes; i++)
	{
		const auto mesh = scene->mMeshes[node->mMeshes[i]];
		process_mesh(mesh, scene);
	}

	for (size_t i = 0; i < node->mNumChildren; i++)
	{
		process_node(node->mChildren[i], scene);
	}
}

void Scene::process_mesh(const aiMesh *mesh, const aiScene *scene)
{
	HittableList triangle_list;

	std::vector<glm::vec3> vertices;
	vertices.reserve(mesh->mNumVertices);

	std::shared_ptr<Material> material;
	if (mesh->mMaterialIndex >= 0)
	{
		const aiMaterial *ai_material = scene->mMaterials[mesh->mMaterialIndex];
		material                      = process_material(ai_material);
	}

	for (size_t i = 0; i < mesh->mNumVertices; i++)
	{
		vertices.emplace_back(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
	}

	for (size_t i = 0; i < mesh->mNumFaces; i++)
	{
		const auto &face = mesh->mFaces[i];
		if (face.mNumIndices != 3)
		{
			LOGE("Face is not a triangle")
			continue;
		}

		const auto &v0 = vertices[face.mIndices[0]];
		const auto &v1 = vertices[face.mIndices[1]];
		const auto &v2 = vertices[face.mIndices[2]];

		triangle_list.add(std::make_shared<Triangle>(v0, v1, v2, material));
	}
}

std::shared_ptr<Material> Scene::process_material(const aiMaterial *material)
{
	return nullptr;
}
}        // namespace mengze::rt

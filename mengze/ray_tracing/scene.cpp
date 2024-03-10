#include "ray_tracing/scene.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <tinyxml2.h>

#include "core/logging.h"
#include "ray_tracing/bvh.h"
#include "ray_tracing/triangle.h"

namespace mengze::rt
{
void HittableList::add(const std::shared_ptr<Hittable> &object)
{
	objects_.push_back(object);
	box_ = Aabb(box_, object->bounding_box());
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

const std::vector<std::shared_ptr<Hittable>> &HittableList::objects() const
{
	return objects_;
}

Aabb HittableList::bounding_box() const
{
	return box_;
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

void Scene::parse_xml(const std::string &file_path)
{
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError    result = doc.LoadFile(file_path.c_str());

	if (result != tinyxml2::XML_SUCCESS)
	{
		LOGE("Failed to load xml file: {}", file_path)
		return;
	}

	glm::vec3 position;
	glm::vec3 look_at;
	glm::vec3 up;
	float     fov;
	int       width;
	int       height;

	tinyxml2::XMLElement *p_camera = doc.FirstChildElement("camera");
	if (p_camera != nullptr)
	{
		p_camera->QueryIntAttribute("width", &width);
		p_camera->QueryIntAttribute("height", &height);
		p_camera->QueryFloatAttribute("fovy", &fov);

		tinyxml2::XMLElement *pEye = p_camera->FirstChildElement("eye");
		if (pEye != nullptr)
		{
			pEye->QueryFloatAttribute("x", &position.x);
			pEye->QueryFloatAttribute("y", &position.y);
			pEye->QueryFloatAttribute("z", &position.z);
		}

		tinyxml2::XMLElement *pLookat = p_camera->FirstChildElement("lookat");
		if (pLookat != nullptr)
		{
			pLookat->QueryFloatAttribute("x", &look_at.x);
			pLookat->QueryFloatAttribute("y", &look_at.y);
			pLookat->QueryFloatAttribute("z", &look_at.z);
		}

		tinyxml2::XMLElement *pUp = p_camera->FirstChildElement("up");
		if (pUp != nullptr)
		{
			pUp->QueryFloatAttribute("x", &up.x);
			pUp->QueryFloatAttribute("y", &up.y);
			pUp->QueryFloatAttribute("z", &up.z);
		}
	}

	camera_ = std::make_shared<Camera>(position, look_at, up, fov);
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
	LOGI("start")
	/*std::shared_ptr<HittableList> triangle_list = std::make_shared<HittableList>();*/

	std::vector<std::shared_ptr<Hittable>> triangles;

	std::vector<glm::vec3> vertices;
	vertices.reserve(mesh->mNumVertices);

	std::shared_ptr<Material> material;
	if (mesh->mMaterialIndex >= 0)
	{
		const aiMaterial *ai_material = scene->mMaterials[mesh->mMaterialIndex];
		material                      = process_material(ai_material);
	}

	if (mesh->mNumVertices > 20)
	{
		return;
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

		// triangle_list->add(std::make_shared<Triangle>(v0, v1, v2, material));
		/*add(std::make_shared<Triangle>(v0, v1, v2, material));
		if (material->is_light())
		{
		    add_light(std::make_shared<Triangle>(v0, v1, v2, material));
		}*/

		triangles.push_back(std::make_shared<Triangle>(v0, v1, v2, material));
	}

	/*add(triangle_list);
	if (material->is_light())
	{
	    add_light(triangle_list);
	}*/

	if (triangles.size() > 0)
	{
		auto bvh_tree = std::make_shared<BvhNode>(triangles, 0, triangles.size());
		add(bvh_tree);

		if (material->is_light())
		{
		    add_light(bvh_tree);
		}
		/*for (const auto &triangle : triangles)
		{
		    if (material->is_light())
		    {
		        add_light(triangle);
		    }
		}*/
	}

	else
	{
		for (const auto &triangle : triangles)
		{
			add(triangle);
			if (material->is_light())
			{
				add_light(triangle);
			}
		}
	}

	LOGI("end")
}

std::shared_ptr<Material> Scene::process_material(const aiMaterial *ai_material)
{
	aiString name;
	ai_material->Get(AI_MATKEY_NAME, name);

	if (auto material = material_library_.get(name.C_Str()))
	{
		return material;
	}

	const std::string mat_name = name.C_Str();

	if (mat_name == "Light")
	{
		return std::make_shared<DiffuseLight>(glm::vec3(34.0f, 24.0f, 8.0f));
	}

	aiColor3D color;
	if (AI_SUCCESS == ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, color))
	{
		return std::make_shared<Lambertian>(glm::vec3(color.r, color.g, color.b));
	}
}
}        // namespace mengze::rt

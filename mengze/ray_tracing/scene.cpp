#include "scene.h"

namespace mengze::rt
{
void Scene::add(std::shared_ptr<Hittable> object)
{
	objects_.push_back(object);
}

bool Scene::hit(const Ray &r, Interval ray_t, HitRecord &rec) const
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
}        // namespace mengze

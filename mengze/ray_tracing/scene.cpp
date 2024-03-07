#include "ray_tracing/scene.h"

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

void Scene::add(const std::shared_ptr<Hittable> &object)
{
	world_.add(object);
}

void Scene::add_light(const std::shared_ptr<Hittable> &light)
{
	lights_.add(light);
}
}        // namespace mengze::rt

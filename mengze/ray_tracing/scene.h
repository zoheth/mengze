#pragma once
#include <vector>
#include <memory>

#include "hittable.h"

namespace mengze::rt
{

class Scene : public Hittable
{
public:

	void add(std::shared_ptr<Hittable> object);

	bool hit(const Ray &r, Interval ray_t, HitRecord &rec) const override;

private:
	std::vector<std::shared_ptr<Hittable>> objects_;
};
}

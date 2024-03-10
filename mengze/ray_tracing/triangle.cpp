#include "ray_tracing/triangle.h"

namespace mengze::rt
{
Triangle::Triangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, const std::shared_ptr<Material> &material) :
    v0_(v0), v1_(v1), v2_(v2), material_(material)
{
	normal_ = glm::normalize(glm::cross(v1_ - v0_, v2_ - v0_));
	area_   = 0.5f * glm::length(glm::cross(v1_ - v0_, v2_ - v0_));
	set_bounding_box();
}

bool Triangle::hit(const Ray &r, Interval ray_t, HitRecord &rec) const
{
	constexpr float epsilon = 1e-8f;

	glm::vec3 edge1 = v1_ - v0_;
	glm::vec3 edge2 = v2_ - v0_;
	glm::vec3 h     = glm::cross(r.direction(), edge2);
	float     a     = glm::dot(edge1, h);

	if (a > -epsilon && a < epsilon)
		return false;

	float     f = 1.0f / a;
	glm::vec3 s = r.origin() - v0_;
	float     u = f * glm::dot(s, h);

	if (u < 0.0f || u > 1.0f)
		return false;

	glm::vec3 q = glm::cross(s, edge1);
	float     v = f * glm::dot(r.direction(), q);

	if (v < 0.0f || u + v > 1.0f)
		return false;

	float t = f * glm::dot(edge2, q);

	if (t < ray_t.min() || t > ray_t.max())
		return false;

	rec.t        = t;
	rec.position = r.at(t);
	rec.material = material_;
	rec.set_face_normal(r, normal_);
	return true;
}

float Triangle::pdf_value(const glm::vec3 &origin, const glm::vec3 &direction) const
{
	HitRecord rec;
	if (!hit(Ray(origin, direction), Interval{0.001f, std::numeric_limits<float>::max()}, rec))
		return 0.0f;

	auto distance_squared = rec.t * rec.t * glm::dot(direction, direction);
	auto cosine           = std::fabs(glm::dot(direction, rec.normal) / glm::length(direction));

	return distance_squared / (cosine * area_);
}

glm::vec3 Triangle::random(const glm::vec3 &origin) const
{
	float r1 = random_float();
	float r2 = random_float();

	if (r1 + r2 >= 1.0f)
	{
		r1 = 1.0f - r1;
		r2 = 1.0f - r2;
	}

	auto random_point = v0_ + r1 * (v1_ - v0_) + r2 * (v2_ - v0_);
	return random_point - origin;
}

Aabb Triangle::bounding_box() const
{
	return b_box_;
}

void Triangle::set_bounding_box()
{
	glm::vec3 min_v = glm::min(v0_, glm::min(v1_, v2_));

	glm::vec3 max_v = glm::max(v0_, glm::max(v1_, v2_));

	b_box_ = Aabb(min_v, max_v);
}
}        // namespace mengze::rt

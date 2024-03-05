#include "material.h"

#include "util/math.h"
#include "hittable.h"

namespace mengze::rt
{
bool Lambertian::scatter(const Ray &ray_in, const HitRecord &rec, glm::vec3 &attenuation, Ray &scattered) const
{
	auto scatter_direction = rec.normal + random_unit_vector();

	if (near_zero(scatter_direction))
	{
		scatter_direction = rec.normal;
	}

	scattered   = Ray(rec.position, scatter_direction);
	attenuation = albedo_->value(rec.u, rec.v, rec.position);

	return true;
}

bool Metal::scatter(const Ray &ray_in, const HitRecord &rec, glm::vec3 &attenuation, Ray &scattered) const
{
	auto reflected = glm::reflect(glm::normalize(ray_in.direction()), rec.normal);
	scattered      = Ray(rec.position, reflected + fuzz_ * random_unit_vector());
	attenuation    = albedo_;

	return glm::dot(scattered.direction(), rec.normal) > 0;
}

bool Dielectric::scatter(const Ray &ray_in, const HitRecord &rec, glm::vec3 &attenuation, Ray &scattered) const
{
	attenuation            = glm::vec3(1.0, 1.0, 1.0);
	float refraction_ratio = rec.front_face ? (1.0f / refraction_index_) : refraction_index_;

	glm::vec3 unit_direction = glm::normalize(ray_in.direction());
	float     cos_theta      = std::min(glm::dot(-unit_direction, rec.normal), 1.0f);
	float     sin_theta      = std::sqrt(1.0f - cos_theta * cos_theta);

	bool      cannot_refract = refraction_ratio * sin_theta > 1.0f;
	glm::vec3 direction;

	if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_float(0, 1))
	{
		direction = glm::reflect(unit_direction, rec.normal);
	}
	else
	{
		direction = glm::refract(unit_direction, rec.normal, refraction_ratio);
	}

	scattered = Ray(rec.position, direction);
	return true;
}
}        // namespace mengze

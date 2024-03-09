#include "material.h"

#include "ray_tracing/math.h"
#include "hittable.h"

namespace mengze::rt
{
bool Lambertian::scatter(const Ray &ray_in, const HitRecord &hit_record, ScatterRecord &scatter_record) const
{
	scatter_record.attenuation = albedo_->value(hit_record.u, hit_record.v, hit_record.position);
	scatter_record.skip_pdf = false;
	scatter_record.pdf = std::make_shared<CosinePdf>(hit_record.normal);

	return true;
}

float Lambertian::scattering_pdf(const Ray &ray_in, const HitRecord &hit_record, const Ray &scattered) const
{
	auto cosine = glm::dot(hit_record.normal, glm::normalize(scattered.direction()));
	return cosine < 0 ? 0 : cosine / M_PI;
}

bool Metal::scatter(const Ray &ray_in, const HitRecord &hit_record, ScatterRecord &scatter_record) const
{
	scatter_record.attenuation = albedo_;
	scatter_record.skip_pdf = true;
	scatter_record.pdf         = nullptr;
	glm::vec3 reflected        = glm::reflect(glm::normalize(ray_in.direction()), hit_record.normal);
	scatter_record.skip_pdf_ray = Ray(hit_record.position, reflected + fuzz_ * random_in_unit_sphere());

	return true;
}

bool Dielectric::scatter(const Ray &ray_in, const HitRecord &hit_record, ScatterRecord &scatter_record) const
{
	scatter_record.attenuation = glm::vec3(1.0, 1.0, 1.0);
	float refraction_ratio     = hit_record.front_face ? (1.0f / refraction_index_) : refraction_index_;
	scatter_record.skip_pdf    = true;
	scatter_record.pdf         = nullptr;

	glm::vec3 unit_direction = glm::normalize(ray_in.direction());
	float     cos_theta      = std::min(glm::dot(-unit_direction, hit_record.normal), 1.0f);
	float     sin_theta      = std::sqrt(1.0f - cos_theta * cos_theta);

	bool      cannot_refract = refraction_ratio * sin_theta > 1.0f;
	glm::vec3 direction;

	if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_float(0, 1))
	{
		direction = glm::reflect(unit_direction, hit_record.normal);
	}
	else
	{
		direction = glm::refract(unit_direction, hit_record.normal, refraction_ratio);
	}

	scatter_record.skip_pdf_ray = Ray(hit_record.position, direction);
	return true;
}

std::shared_ptr<Material> MaterialLibrary::get(const std::string &name) const
{
	const auto it = materials_.find(name);
	if (it != materials_.end())
	{
		return it->second;
	}
	return nullptr;
}

void MaterialLibrary::add(const std::string &name, const std::shared_ptr<Material> &material)
{
	materials_[name] = material;
}
}        // namespace mengze

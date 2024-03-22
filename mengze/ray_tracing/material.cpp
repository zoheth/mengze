#include "material.h"

#include "hittable.h"
#include "core/logging.h"
#include "ray_tracing/math.h"
#include "ray_tracing/pdf.h"

namespace mengze::rt
{

float rgb_to_luminance(glm::vec3 rgb)
{
	return 0.2126f * rgb.r + 0.7152f * rgb.g + 0.0722 * rgb.b;
}

bool Lambertian::scatter(const Ray &ray_in, const HitRecord &hit_record, ScatterRecord &scatter_record) const
{
	scatter_record.attenuation = albedo_->value(hit_record.u, hit_record.v, hit_record.position);
	scatter_record.skip_pdf    = false;
	scatter_record.pdf         = std::make_shared<CosinePdf>(hit_record.normal);

	return true;
}

glm::vec3 Lambertian::debug_color(float u, float v, const glm::vec3 &p) const
{
	return albedo_->value(u, v, p);
}


float Lambertian::scattering_pdf(const Ray &ray_in, const HitRecord &hit_record, const Ray &scattered) const
{
	auto cosine = glm::dot(hit_record.normal, glm::normalize(scattered.direction()));
	return cosine < 0 ? 0 : cosine / glm::pi<float>();
}

float Metal::shininess_to_fuzz(float shininess)
{
	float s    = 2000.0f;
	float fuzz = exp(-shininess / s);

	fuzz = glm::clamp(fuzz, 0.0f, 1.0f);
	return fuzz;
}

bool Metal::scatter(const Ray &ray_in, const HitRecord &hit_record, ScatterRecord &scatter_record) const
{
	scatter_record.attenuation  = albedo_;
	scatter_record.skip_pdf     = true;
	scatter_record.pdf          = nullptr;
	glm::vec3 reflected         = glm::reflect(glm::normalize(ray_in.direction()), hit_record.normal);
	scatter_record.skip_pdf_ray = Ray(hit_record.position, reflected + fuzz_ * random_in_unit_sphere());

	return true;
}

bool PhongMaterial::scatter(const Ray &ray_in, const HitRecord &hit_record, ScatterRecord &scatter_record) const
{
	scatter_record.attenuation = specular_texture_->value(hit_record.u, hit_record.v, hit_record.position);
	scatter_record.skip_pdf    = false;
	float ks                   = rgb_to_luminance(specular_texture_->value(hit_record.u, hit_record.v, hit_record.position));
	float kd                   = rgb_to_luminance(diffuse_texture_->value(hit_record.u, hit_record.v, hit_record.position));
	scatter_record.pdf         = std::make_shared<PhongPdf>(hit_record.normal, glm::reflect(glm::normalize(ray_in.direction()), hit_record.normal), shininess_, kd, ks);

	return true;
}

float PhongMaterial::scattering_pdf(const Ray &ray_in, const HitRecord &hit_record, const Ray &scattered) const
{
	float ks = rgb_to_luminance(specular_texture_->value(hit_record.u, hit_record.v, hit_record.position));
	float kd = rgb_to_luminance(diffuse_texture_->value(hit_record.u, hit_record.v, hit_record.position));

	glm::vec3 normalized_direction = glm::normalize(scattered.direction());

	float cos_theta = glm::dot(normalized_direction, hit_record.normal);
	if (cos_theta <= 0)
		return 0;

	float diffuse = kd / glm::pi<float>();

	glm::vec3 reflected = glm::reflect(-normalized_direction, hit_record.normal);

	
	float cos_alpha = glm::dot(glm::normalize(reflected), glm::normalize(ray_in.direction()));
	cos_alpha       = glm::clamp(cos_alpha, 0.0f, 1.0f);

	float specular = ks * (shininess_ + 2) / (2 * glm::pi<float>()) * std::pow(cos_alpha, shininess_);

	float pdf = diffuse + specular;
	return pdf;
}

glm::vec3 PhongMaterial::debug_color(float u, float v, const glm::vec3 &p) const
{
	return diffuse_texture_->value(u, v, p);
}

float PhongMaterial::reflectance(float cosine, float shininess)
{
	// Adjust the model as needed to get a reasonable behavior for your scene
	float base = 0.5 + 0.5 * pow(cosine, shininess);
	return glm::clamp(base, 0.0f, 1.0f);
}

glm::vec3 PhongMaterial::fuzz(float shininess)
{
	return glm::vec3(1.0f - glm::clamp(shininess / 5000.0f, 0.0f, 1.0f));
}

Dielectric::Dielectric(float refraction_index):
	refraction_index_(refraction_index)
{}

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
}        // namespace mengze::rt

#pragma once

#include <memory>

#include "ray.h"
#include "texture.h"


namespace mengze::rt
{

class HitRecord;

class Material
{
  public:
	virtual ~Material() = default;

	virtual bool scatter(const Ray &ray_in, const HitRecord &rec, glm::vec3 &attenuation, Ray &scattered) const = 0;

	virtual glm::vec3 emitted(float u, float v, const glm::vec3 &p) const
	{
		return glm::vec3(0.0f);
	}
};

class Lambertian : public Material
{
  public:
	Lambertian(const std::shared_ptr<Texture> &albedo) :
	    albedo_(albedo)
	{}

	Lambertian(const glm::vec3 &albedo) :
	    albedo_(std::make_shared<SolidColor>(albedo))
	{}

	bool scatter(const Ray &ray_in, const HitRecord &rec, glm::vec3 &attenuation, Ray &scattered) const override;

  private:
	std::shared_ptr<Texture> albedo_;
};

class Metal : public Material
{
  public:
	Metal(const glm::vec3 &albedo, float fuzz) :
	    albedo_(albedo),
	    fuzz_(fuzz < 1.0 ? fuzz : 1.0)
	{}

	bool scatter(const Ray &ray_in, const HitRecord &rec, glm::vec3 &attenuation, Ray &scattered) const override;

  private:
	glm::vec3 albedo_;
	float     fuzz_;
};

class Dielectric : public Material
{
	  public:
	Dielectric(float refraction_index) :
	    refraction_index_(refraction_index)
	{}

	bool scatter(const Ray &ray_in, const HitRecord &rec, glm::vec3 &attenuation, Ray &scattered) const override;

  private:
	float refraction_index_;

	static float reflectance(float cosine, float ref_idx)
	{
		// Use Schlick's approximation for reflectance.
		auto r0 = (1 - ref_idx) / (1 + ref_idx);
		r0      = r0 * r0;
		return r0 + (1 - r0) * pow((1 - cosine), 5);
	}
};

class DiffuseLight : public Material
{
public:
	explicit DiffuseLight(const std::shared_ptr<Texture> &emit) : emit_(emit) {}
	explicit DiffuseLight(const glm::vec3 &color) : emit_(std::make_shared<SolidColor>(color)) {}

	bool scatter(const Ray &ray_in, const HitRecord &rec, glm::vec3 &attenuation, Ray &scattered) const override
	{
		return false;
	}

	glm::vec3 emitted(float u, float v, const glm::vec3 &p) const override
	{
		return emit_->value(u, v, p);
	}

private:
	std::shared_ptr<Texture> emit_;
};

}        // namespace mengze

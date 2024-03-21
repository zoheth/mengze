#pragma once
#include "ray_tracing/math.h"

namespace mengze::rt
{
class Pdf
{
  public:
	virtual ~Pdf() = default;

	virtual float value(const glm::vec3 &direction) const = 0;

	virtual glm::vec3 generate() const = 0;
};

class CosinePdf : public Pdf
{
  public:
	CosinePdf(const glm::vec3 &w);

	float value(const glm::vec3 &direction) const override;

	glm::vec3 generate() const override;

  private:
	OrthoNormalBasis uvw_;
};

class MixturePdf : public Pdf
{
  public:
	MixturePdf(std::shared_ptr<Pdf> p0, std::shared_ptr<Pdf> p1);

	float value(const glm::vec3 &direction) const override;

	glm::vec3 generate() const override;

  private:
	std::shared_ptr<Pdf> p0_;
	std::shared_ptr<Pdf> p1_;
};

class PhongPdf : public Pdf
{
  public:
	PhongPdf(const glm::vec3 &w, const glm::vec3 &reflect_dir, float ns, float kd, float ks);

	float value(const glm::vec3 &direction) const override;

	glm::vec3 generate() const override;

	glm::vec3 random_phong_specular_direction() const;

  private:
	OrthoNormalBasis uvw_;
	glm::vec3        reflect_dir_;
	float            ns_;
	float            kd_;
	float            ks_;
};
}

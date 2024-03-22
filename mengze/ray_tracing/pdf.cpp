#include "pdf.h"

namespace mengze::rt
{

CosinePdf::CosinePdf(const glm::vec3 &w)
{
	uvw_.build_from_w(w);
}

float CosinePdf::value(const glm::vec3 &direction) const
{
	auto cosine = glm::dot(glm::normalize(direction), uvw_.w());
	return (cosine <= 0) ? 0 : cosine / glm::pi<float>();
}

glm::vec3 CosinePdf::generate() const
{
	return uvw_.to_local(random_cosine_direction());
}

MixturePdf::MixturePdf(std::shared_ptr<Pdf> p0, std::shared_ptr<Pdf> p1, float wight) :
    p0_(std::move(p0)), p1_(std::move(p1)), wight_(wight)
{}

float MixturePdf::value(const glm::vec3 &direction) const
{
	return wight_ * p0_->value(direction) + (1-wight_) * p1_->value(direction);
}

glm::vec3 MixturePdf::generate() const
{
	if (random_float() < wight_)
		return p0_->generate();
	else
		return p1_->generate();
}

PhongPdf::PhongPdf(const glm::vec3 &w, const glm::vec3 &reflect_dir, float ns, float kd, float ks)
	:reflect_dir_(reflect_dir), ns_(ns), kd_(kd), ks_(ks)
{
	uvw_.build_from_w(w);
}

float PhongPdf::value(const glm::vec3 &direction) const
{
	glm::vec3 normalized_direction = glm::normalize(direction);

	float cos_theta = glm::dot(normalized_direction, uvw_.w());
	if (cos_theta <= 0)
		return 0;

	float diffuse = kd_ / glm::pi<float>();

	glm::vec3 reflected = glm::reflect(-normalized_direction, uvw_.w());

	float cos_alpha = glm::dot(glm::normalize(reflected), reflect_dir_);
	cos_alpha = glm::clamp(cos_alpha, 0.0f, 1.0f);

	float specular = ks_ * (ns_ + 2) / (2 * glm::pi<float>()) * pow(cos_alpha, ns_);

	return diffuse + specular;
}


glm::vec3 PhongPdf::generate() const
{
	if (random_float() < kd_ / (kd_ + ks_))
	{
		// Sample diffuse component
		return uvw_.to_local(random_cosine_direction());
	}
	else
	{
		// Sample specular component
		return random_phong_specular_direction();
	}
}

glm::vec3 align_direction_with(const glm::vec3 &local_direction, const glm::vec3 &world_direction)
{
	// Build a local coordinate system where 'world_direction' is the new 'Z' axis
	glm::vec3 w = glm::normalize(world_direction);
	glm::vec3 a = (fabs(w.x) > 0.9f) ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
	glm::vec3 v = glm::normalize(glm::cross(w, a));
	glm::vec3 u = glm::cross(w, v);

	// Transform 'local_direction' from the local space to world space
	return local_direction.x * u + local_direction.y * v + local_direction.z * w;
}

glm::vec3 PhongPdf::random_phong_specular_direction() const
{
	// Step 1: Generate two random numbers
	float u1 = random_float();
	float u2 = random_float();

	// Step 2: Convert uniform random numbers to concentration around specular direction
	float phi = 2 * glm::pi<float>() * u1;        // Full circle
	// The polar angle is derived from u2, adjusted by the shininess exponent to concentrate the distribution
	float cos_theta = pow(u2, 1.0f / (ns_ + 1));
	float sin_theta = sqrt(1.0f - cos_theta * cos_theta);

	// Step 3: Convert spherical coordinates (phi, theta) to Cartesian coordinates
	glm::vec3 local_direction = glm::vec3(
	    sin_theta * cos(phi),        // X
	    sin_theta * sin(phi),        // Y
	    cos_theta                    // Z, aligned with the specular direction in local space
	);

	// Assuming reflect_dir_ is your specular direction in world space and you have a method to transform
	// a direction from local space to be aligned with this specular direction
	glm::vec3 world_direction = align_direction_with(local_direction, reflect_dir_);

	return world_direction;
}
}

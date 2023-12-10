#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace mengze
{
	enum class CameraMode
	{
		ROAMING,
		ORBIT
	};
	class Camera
	{
	public:
		Camera(float fov, float near, float far, float speed = 1.0f);

		void on_update(float delta_time);
		void on_resize(uint32_t width, uint32_t height);

		glm::mat4 get_projection_matrix() const { return projection_matrix_; }

		glm::mat4 get_view_matrix() const { return view_matrix_; }

		float get_near() const { return near_; }
		float get_far() const { return far_; }

	private:
		void roaming_update(float delta_time);
		void orbit_update(float delta_time);
		void update_projection_matrix();
		void update_view_matrix();
	private:
		glm::mat4 projection_matrix_{ 1.0f };
		glm::mat4 view_matrix_{ 1.0f };
		glm::mat4 inverse_projection_matrix_{ 1.0f };
		glm::mat4 inverse_view_matrix_{ 1.0f };

		float fov_{ 45.0f };
		float near_{ 0.1f };
		float far_{ 1000.0f };
		float speed_{ 1.0f };

		CameraMode mode_{ CameraMode::ROAMING };

		glm::vec3 position_{ 0.0f , 0.1f, 3.5f };
		glm::vec3 up_direction_{ 0.0f, 1.0f, 0.0f };
		glm::vec3 forward_direction_{ 0.0f , 0.0f,-1.0f };

		glm::vec2 last_mouse_position_{ 0.0f };

		uint32_t viewport_width_{ 0 };
		uint32_t viewport_height_{ 0 };
	};
}
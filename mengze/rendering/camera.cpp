#include "camera.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "core/input/input.h"

namespace mengze
{
	Camera::Camera(float fov, float near, float far, float speed)
		: fov_(fov), near_(near), far_(far), speed_(speed)
	{
		update_view_matrix();
	}

	void Camera::on_update(float delta_time)
	{
		if(mode_ == CameraMode::ROAMING)
			roaming_update(delta_time);
		else if (mode_ == CameraMode::ORBIT)
			orbit_update(delta_time);
		
	}

	void Camera::on_resize(uint32_t width, uint32_t height)
	{
		if (viewport_width_ == width && viewport_height_ == height)
			return;

		viewport_width_ = width;
		viewport_height_ = height;

		update_projection_matrix();
	}

	void Camera::roaming_update(float delta_time)
	{
		glm::vec2 mouse_pos = Input::get_mouse_position();
		glm::vec2 delta = (mouse_pos - last_mouse_position_) * 0.05f;
		last_mouse_position_ = mouse_pos;

		if (!Input::is_mouse_button_pressed(MouseButton::Right))
		{
			Input::set_mouse_cursor(CursorMode::Normal);
			return;
		}

		Input::set_mouse_cursor(CursorMode::Locked);

		bool moved = false;
		const glm::vec3 right_direction = glm::normalize(glm::cross(forward_direction_, up_direction_));

		if (Input::is_key_pressed(Key::W))
		{
			position_ += forward_direction_ * speed_ * delta_time;
			moved = true;
		}
		else if (Input::is_key_pressed(Key::S))
		{
			position_ -= forward_direction_ * speed_ * delta_time;
			moved = true;
		}
		if (Input::is_key_pressed(Key::A))
		{
			position_ -= right_direction * speed_ * delta_time;
			moved = true;
		}
		else if (Input::is_key_pressed(Key::D))
		{
			position_ += right_direction * speed_ * delta_time;
			moved = true;
		}
		if (Input::is_key_pressed(Key::Q))
		{
			position_ -= up_direction_ * speed_ * delta_time;
			moved = true;
		}
		else if (Input::is_key_pressed(Key::E))
		{
			position_ += up_direction_ * speed_ * delta_time;
			moved = true;
		}

		if (delta.x != 0.0f || delta.y != 0.0f)
		{
			moved = true;
			glm::quat yaw = glm::angleAxis(glm::radians(-delta.x), up_direction_);
			glm::quat pitch = glm::angleAxis(glm::radians(delta.y), right_direction);

			forward_direction_ = glm::normalize(yaw * forward_direction_ * pitch);

			moved = true;
		}

		if (moved)
		{
			update_view_matrix();
		}
	}

	void Camera::orbit_update(float delta_time)
	{
		
	}

	void Camera::update_projection_matrix()
	{
		projection_matrix_ = glm::perspective(glm::radians(fov_), static_cast<float>(viewport_width_) / static_cast<float>(viewport_height_), near_, far_);
		inverse_projection_matrix_ = glm::inverse(projection_matrix_);
	}

	void Camera::update_view_matrix()
	{
		view_matrix_ = glm::lookAt(position_, position_ + forward_direction_, up_direction_);
		inverse_view_matrix_ = glm::inverse(view_matrix_);
	}
}

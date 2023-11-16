#include "input.h"

#include "core/application.h"
#include "GLFW/glfw3.h"

namespace mengze
{
	bool Input::is_key_pressed(KeyCode key_code)
	{
		GLFWwindow* window = Application::get().get_window_handle();
		int state = glfwGetKey(window, static_cast<int32_t>(key_code));
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::is_mouse_button_pressed(MouseButton mouse_button)
	{
		GLFWwindow* window = Application::get().get_window_handle();
		int state = glfwGetMouseButton(window, static_cast<int32_t>(mouse_button));
		return state == GLFW_PRESS;
	}

	glm::vec2 Input::get_mouse_position()
	{
		GLFWwindow* window = Application::get().get_window_handle();
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		return { x, y };
	}

	void Input::set_mouse_cursor(CursorMode mode)
	{
		GLFWwindow* window = Application::get().get_window_handle();
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL + static_cast<int32_t>(mode));
	}
}

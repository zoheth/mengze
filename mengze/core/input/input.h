#pragma once

#include <glm/glm.hpp>

#include "key_codes.h"

namespace mengze
{
	class Input
	{
	public:
		static bool is_key_pressed(KeyCode key_code);
		static bool is_mouse_button_pressed(MouseButton mouse_button);

		static glm::vec2 get_mouse_position();

		static void set_mouse_cursor(CursorMode mode);
	};
}
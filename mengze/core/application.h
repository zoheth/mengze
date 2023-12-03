#pragma once

#include <functional>
#include <string>
#include <memory>
#include <vector>

#include <volk.h>

#include "layer.h"

void check_vk_result(VkResult err);

struct GLFWwindow;

namespace mengze {
	struct ApplicationSpecification {
		std::string name = "App";
		uint32_t width = 1600;
		uint32_t height = 900;
	};


	class Application
	{
	public:
		Application(ApplicationSpecification spec = ApplicationSpecification{});
		~Application();

		static Application& get();

		void init();
		void run();

		static float get_time();

		void shutdown();

		/**
		 * Adds a new layer of type T to the application.
		 *
		 * Usage:
		 *   push_layer<LayerType>(constructor_arg1, constructor_arg2, ...);
		 * where LayerType is a subclass of Layer and constructor_arg1, constructor_arg2, ...,
		 * are the arguments required by the constructor of LayerType.
		 */
		template<typename T, typename... Args>
		Layer* push_layer(Args&&... args)
		{
			static_assert(std::is_base_of_v<Layer, T>, "Pushed type is not subclass of Layer!");
			layers_.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
			layers_.back()->on_attach();

			return layers_.back().get();
		}

		GLFWwindow* get_window_handle() const { return window_; }

		static VkInstance get_instance();
		static VkDevice get_device();
		static VkPhysicalDevice get_physical_device();

		static VkCommandBuffer get_command_buffer(bool begin);
		static void flush_command_buffer(VkCommandBuffer command_buffer);

		static void submit_resource_free(const std::function<void()>& func);

	private:
		ApplicationSpecification spec_;
		GLFWwindow* window_{ nullptr };
		bool running_{ false };

		float last_frame_time_{ 0.0f };
		float frame_time_{ 0.0f };
		float time_step_{ 0.0f };

		std::vector<std::unique_ptr<Layer>> layers_;
	};

	std::unique_ptr<Application> create_application(int argc, char** argv);
}
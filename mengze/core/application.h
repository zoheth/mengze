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

		void shutdown();

		template<typename T>
		void push_layer()
		{
			static_assert(std::is_base_of<Layer, T>::value, "Pushed type is not subclass of Layer!");
			layers_.emplace_back(std::make_unique<T>());
			layers_.back()->on_attach();
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

		std::vector<std::unique_ptr<Layer>> layers_;
	};

	std::unique_ptr<Application> create_application(int argc, char** argv);
}
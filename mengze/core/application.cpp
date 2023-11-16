#include "application.h"

#include <corecrt_io.h>
#include <functional>

#include <volk.h>
#include "GLFW/glfw3.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include "logging.h"


void check_vk_result(VkResult err)
{
	if (err == 0)
		return;
	LOGE("[vulkan] Error: VkResult = {}", err)
		if (err < 0)
			abort();
}

namespace
{
	VkAllocationCallbacks* s_allocator = nullptr;
	VkInstance s_instance = VK_NULL_HANDLE;
	VkPhysicalDevice s_physical_device = VK_NULL_HANDLE;
	VkDevice s_device = VK_NULL_HANDLE;
	uint32_t s_queue_family = static_cast<uint32_t>(-1);
	VkQueue s_queue = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT s_debug_utils = VK_NULL_HANDLE;
	VkPipelineCache s_pipeline_cache = VK_NULL_HANDLE;
	VkDescriptorPool s_descriptor_pool = VK_NULL_HANDLE;

	ImGui_ImplVulkanH_Window s_window;
	int s_min_image_count = 2;
	bool s_swapchain_rebuild = false;

	std::vector<std::vector<VkCommandBuffer>> s_all_command_buffers;
	std::vector<std::vector<std::function<void()>>> s_resource_free_queue;
	uint32_t s_current_frame_index = 0;

	mengze::Application* s_application = nullptr;

	VKAPI_ATTR VkBool32 VKAPI_CALL debug_utils_messenger_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		(void)messageType; // Unused argument
		(void)pUserData;   // Unused argument

		switch (messageSeverity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			LOGD("[vulkan] {}", pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			LOGI("[vulkan] {}", pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			LOGW("[vulkan] {}", pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			LOGE("[vulkan] {}", pCallbackData->pMessage);
			break;
		default:
			LOGE("[vulkan] UNKNOWN: {}", pCallbackData->pMessage);
			break;
		}

		return VK_FALSE;
	}


	void setup_vulkan(const char** extensions, uint32_t extensions_count)
	{
		VkResult err = volkInitialize();
		check_vk_result(err);

		// Create Vulkan Instance
		{
			VkInstanceCreateInfo create_info = {};
			create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			create_info.enabledExtensionCount = extensions_count;
			create_info.ppEnabledExtensionNames = extensions;
#ifdef MZ_DEBUG
			const char* validation_layers[] = { "VK_LAYER_KHRONOS_validation" };
			create_info.enabledLayerCount = 1;
			create_info.ppEnabledLayerNames = validation_layers;

			const auto extensions_ext = static_cast<const char**>(malloc(sizeof(const char*) * (extensions_count + 1)));
			memcpy(extensions_ext, extensions, sizeof(const char*) * extensions_count);
			extensions_ext[extensions_count] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
			create_info.enabledExtensionCount = extensions_count + 1;
			create_info.ppEnabledExtensionNames = extensions_ext;

			err = vkCreateInstance(&create_info, s_allocator, &s_instance);
			check_vk_result(err);
			free(extensions_ext);

			volkLoadInstance(s_instance);

			VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {};
			debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debug_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			debug_create_info.pfnUserCallback = debug_utils_messenger_callback;
			debug_create_info.pUserData = nullptr;
			err = vkCreateDebugUtilsMessengerEXT(s_instance, &debug_create_info, s_allocator, &s_debug_utils);
			check_vk_result(err);
#else
			err = vkCreateInstance(&create_info, s_allocator, &s_instance);
			check_vk_result(err);

			volkLoadInstance(s_instance);
			(void)s_debug_utils;
#endif
		}

		// Select GPU
		{
			uint32_t gpu_count = 0;
			err = vkEnumeratePhysicalDevices(s_instance, &gpu_count, nullptr);
			check_vk_result(err);
			assert(gpu_count > 0);

			const auto gpus = static_cast<VkPhysicalDevice*>(malloc(sizeof(VkPhysicalDevice) * gpu_count));
			err = vkEnumeratePhysicalDevices(s_instance, &gpu_count, gpus);
			check_vk_result(err);

			int selected_gpu = 0;
			for (int i = 0; i < gpu_count; i++)
			{
				VkPhysicalDeviceProperties properties;
				vkGetPhysicalDeviceProperties(gpus[i], &properties);
				if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				{
					selected_gpu = i;
					break;
				}
			}

			s_physical_device = gpus[selected_gpu];
			free(gpus);
		}

		// Find Graphics Queue Family
		{
			uint32_t count;
			vkGetPhysicalDeviceQueueFamilyProperties(s_physical_device, &count, nullptr);
			auto* queues = static_cast<VkQueueFamilyProperties*>(malloc(sizeof(VkQueueFamilyProperties) * count));
			vkGetPhysicalDeviceQueueFamilyProperties(s_physical_device, &count, queues);
			for (uint32_t i = 0; i < count; i++)
			{
				if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					s_queue_family = i;
					break;
				}
			}
			free(queues);
			assert(s_queue_family != static_cast<uint32_t>(-1));
		}

		// Create Vulkan Device
		{
			const char* device_extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
			constexpr float queue_priorities[] = { 1.0f };
			VkDeviceQueueCreateInfo queue_create_info[1] = {};
			queue_create_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_info[0].queueFamilyIndex = s_queue_family;
			queue_create_info[0].queueCount = 1;
			queue_create_info[0].pQueuePriorities = queue_priorities;
			VkDeviceCreateInfo create_info = {};
			create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			create_info.queueCreateInfoCount = std::size(queue_create_info);
			create_info.pQueueCreateInfos = queue_create_info;
			create_info.enabledExtensionCount = std::size(device_extensions);
			create_info.ppEnabledExtensionNames = device_extensions;
			err = vkCreateDevice(s_physical_device, &create_info, s_allocator, &s_device);
			check_vk_result(err);
			vkGetDeviceQueue(s_device, s_queue_family, 0, &s_queue);
		}

		{
			constexpr VkDescriptorPoolSize pool_sizes[] =
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
			};
			VkDescriptorPoolCreateInfo pool_info = {};
			pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
			pool_info.poolSizeCount = static_cast<uint32_t>(IM_ARRAYSIZE(pool_sizes));
			pool_info.pPoolSizes = pool_sizes;
			err = vkCreateDescriptorPool(s_device, &pool_info, s_allocator, &s_descriptor_pool);
			check_vk_result(err);
		}
	}

	void setup_vulkan_window(VkSurfaceKHR surface, int width, int height)
	{
		s_window.Surface = surface;

		VkBool32 res;
		vkGetPhysicalDeviceSurfaceSupportKHR(s_physical_device, s_queue_family, s_window.Surface, &res);
		if (res != VK_TRUE)
		{
			LOGE("[vulkan] No WSI support on physical device 0");
			exit(-1);
		}

		constexpr VkFormat request_surface_image_format[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
		constexpr VkColorSpaceKHR request_surface_color_space = VK_COLORSPACE_SRGB_NONLINEAR_KHR;

		s_window.SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(s_physical_device, s_window.Surface, request_surface_image_format, static_cast<uint32_t>(std::size(request_surface_image_format)), request_surface_color_space);

		VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
		s_window.PresentMode = ImGui_ImplVulkanH_SelectPresentMode(s_physical_device, s_window.Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));

		assert(s_min_image_count >= 2);
		ImGui_ImplVulkanH_CreateOrResizeWindow(s_instance, s_physical_device, s_device, &s_window, s_queue_family, s_allocator, width, height, s_min_image_count);
	}

	void frame_render(ImDrawData* draw_data)
	{
		const VkSemaphore image_acquired_semaphore = s_window.FrameSemaphores[s_window.SemaphoreIndex].ImageAcquiredSemaphore;
		const VkSemaphore render_complete_semaphore = s_window.FrameSemaphores[s_window.SemaphoreIndex].RenderCompleteSemaphore;
		VkResult err = vkAcquireNextImageKHR(s_device, s_window.Swapchain, UINT64_MAX, image_acquired_semaphore,
			VK_NULL_HANDLE, &s_window.FrameIndex);
		if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
		{
			s_swapchain_rebuild = true;
			return;
		}
		check_vk_result(err);

		s_current_frame_index = (s_current_frame_index + 1) % s_window.ImageCount;

		ImGui_ImplVulkanH_Frame* fd = &(s_window.Frames[s_window.FrameIndex]);
		{
			err = vkWaitForFences(s_device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);
			check_vk_result(err);

			err = vkResetFences(s_device, 1, &fd->Fence);
			check_vk_result(err);
		}
		{
			for (auto& func : s_resource_free_queue[s_current_frame_index])
			{
				func();
			}
			s_resource_free_queue[s_current_frame_index].clear();
		}
		{
			// These use s_window.FrameIndex and not s_current_frame_index, because they are submitted to a different queue than the main rendering
			auto& cmd = s_all_command_buffers[s_window.FrameIndex];
			if (!cmd.empty())
			{
				vkFreeCommandBuffers(s_device, fd->CommandPool, static_cast<uint32_t>(cmd.size()), cmd.data());
				cmd.clear();
			}

			err = vkResetCommandPool(s_device, fd->CommandPool, 0);
			check_vk_result(err);

			VkCommandBufferBeginInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
			check_vk_result(err);
		}
		{
			VkRenderPassBeginInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			info.renderPass = s_window.RenderPass;
			info.framebuffer = fd->Framebuffer;
			info.renderArea.extent.width = s_window.Width;
			info.renderArea.extent.height = s_window.Height;
			info.clearValueCount = 1;
			info.pClearValues = &s_window.ClearValue;
			vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
		}

		ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

		vkCmdEndRenderPass(fd->CommandBuffer);

		{
			constexpr VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			VkSubmitInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			info.waitSemaphoreCount = 1;
			info.pWaitSemaphores = &image_acquired_semaphore;
			info.pWaitDstStageMask = &wait_stage;
			info.commandBufferCount = 1;
			info.pCommandBuffers = &fd->CommandBuffer;
			info.signalSemaphoreCount = 1;
			info.pSignalSemaphores = &render_complete_semaphore;

			err = vkEndCommandBuffer(fd->CommandBuffer);
			check_vk_result(err);
			err = vkQueueSubmit(s_queue, 1, &info, fd->Fence);
			check_vk_result(err);
		}
	}

	void frame_present()
	{
		if (s_swapchain_rebuild)
			return;
		const VkSemaphore render_complete_semaphore = s_window.FrameSemaphores[s_window.SemaphoreIndex].RenderCompleteSemaphore;
		VkPresentInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		info.waitSemaphoreCount = 1;
		info.pWaitSemaphores = &render_complete_semaphore;
		info.swapchainCount = 1;
		info.pSwapchains = &s_window.Swapchain;
		info.pImageIndices = &s_window.FrameIndex;
		VkResult err = vkQueuePresentKHR(s_queue, &info);
		if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
		{
			s_swapchain_rebuild = true;
			return;
		}
		check_vk_result(err);
		s_window.SemaphoreIndex = (s_window.SemaphoreIndex + 1) % s_window.ImageCount;
	}

	void glfw_error_callback(int error, const char* description)
	{
		LOGE("[glfw] Error {}: {}", error, description);
	}
}

namespace mengze
{
	Application::Application(ApplicationSpecification spec)
		:spec_(std::move(spec))
	{
		s_application = this;
		init();
	}

	Application::~Application()
	{
		shutdown();

		s_application = nullptr;
	}

	Application& Application::get()
	{
		return *s_application;
	}

	void Application::init()
	{
		glfwSetErrorCallback(glfw_error_callback);
		if (!glfwInit())
		{
			LOGE("[glfw] Failed to initialize!");
			return;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window_ = glfwCreateWindow(spec_.width, spec_.height, spec_.name.c_str(), nullptr, nullptr);

		if (!glfwVulkanSupported())
		{
			LOGE("[glfw] Vulkan is not supported!");
			return;
		}
		uint32_t extension_count = 0;
		const char** extensions = glfwGetRequiredInstanceExtensions(&extension_count);
		setup_vulkan(extensions, extension_count);
		ImGui_ImplVulkan_LoadFunctions([](const char* function_name, void* vulkan_instance) {
			return vkGetInstanceProcAddr(*(static_cast<VkInstance*>(vulkan_instance)), function_name);
			}, &s_instance);

		VkSurfaceKHR surface;
		VkResult err = glfwCreateWindowSurface(s_instance, window_, s_allocator, &surface);
		check_vk_result(err);

		int w, h;
		glfwGetFramebufferSize(window_, &w, &h);
		setup_vulkan_window(surface, w, h);

		s_all_command_buffers.resize(s_window.ImageCount);
		s_resource_free_queue.resize(s_window.ImageCount);

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigViewportsNoAutoMerge = true;
		//io.ConfigViewportsNoTaskBarIcon = true;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.09f, 0.09f, 1.0f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1f, 0.1f, 0.1f, 0.6f);
		//ImGui::StyleColorsClassic();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		ImGui_ImplGlfw_InitForVulkan(window_, true);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = s_instance;
		init_info.PhysicalDevice = s_physical_device;
		init_info.Device = s_device;
		init_info.QueueFamily = s_queue_family;
		init_info.Queue = s_queue;
		init_info.PipelineCache = s_pipeline_cache;
		init_info.DescriptorPool = s_descriptor_pool;
		init_info.Allocator = s_allocator;
		init_info.MinImageCount = s_min_image_count;
		init_info.ImageCount = s_window.ImageCount;
		init_info.CheckVkResultFn = check_vk_result;
		init_info.Subpass = 0;
		ImGui_ImplVulkan_Init(&init_info, s_window.RenderPass);
	}

	void Application::run()
	{
		running_ = true;

		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		ImGuiIO& io = ImGui::GetIO();

		while (!glfwWindowShouldClose(window_) && running_)
		{
			glfwPollEvents();
			for (auto& layer : layers_)
			{
				layer->on_update();
			}

			if (s_swapchain_rebuild)
			{
				int width, height;
				glfwGetFramebufferSize(window_, &width, &height);
				if (width > 0 && height > 0)
				{
					ImGui_ImplVulkan_SetMinImageCount(s_min_image_count);
					ImGui_ImplVulkanH_CreateOrResizeWindow(s_instance, s_physical_device, s_device, &s_window, s_queue_family, s_allocator, width, height, s_min_image_count);
					s_window.FrameIndex = 0;

					s_all_command_buffers.clear();
					s_all_command_buffers.resize(s_window.ImageCount);

					s_swapchain_rebuild = false;
				}
			}

			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			{
				static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

				// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
				// because it would be confusing to have two docking targets within each others.
				ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;

				const ImGuiViewport* viewport = ImGui::GetMainViewport();
				ImGui::SetNextWindowPos(viewport->WorkPos);
				ImGui::SetNextWindowSize(viewport->WorkSize);
				ImGui::SetNextWindowViewport(viewport->ID);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
				window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
				window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

				// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
				// and handle the pass-thru hole, so we ask Begin() to not render a background.
				if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
					window_flags |= ImGuiWindowFlags_NoBackground;

				// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
				// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
				// all active windows docked into it will lose their parent and become undocked.
				// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
				// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
				ImGui::Begin("DockSpace Demo", nullptr, window_flags);
				ImGui::PopStyleVar();

				ImGui::PopStyleVar(2);

				// Submit the DockSpace
				ImGuiIO& io = ImGui::GetIO();
				if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
				{
					ImGuiID dockspace_id = ImGui::GetID("VulkanAppDockspace");
					ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
				}

				for (const auto& layer:layers_)
					layer->on_ui_render();
				ImGui::End();
			}

			ImGui::Render();
			ImDrawData* draw_data = ImGui::GetDrawData();
			const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
			s_window.ClearValue.color.float32[0] = clear_color.x * clear_color.w;
			s_window.ClearValue.color.float32[1] = clear_color.y * clear_color.w;
			s_window.ClearValue.color.float32[2] = clear_color.z * clear_color.w;
			s_window.ClearValue.color.float32[3] = clear_color.w;
			if (!is_minimized)
				frame_render(draw_data);

			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
			}

			if (!is_minimized)
				frame_present();

		}
	}

	void Application::shutdown()
	{
		for (auto& layer : layers_)
		{
			layer->on_detach();
		}

		layers_.clear();

		VkResult err = vkDeviceWaitIdle(s_device);
		check_vk_result(err);

		for(auto& queue : s_resource_free_queue)
		{
			for (auto& func : queue)
				func();
		}
		s_resource_free_queue.clear();

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		ImGui_ImplVulkanH_DestroyWindow(s_instance, s_device, &s_window, s_allocator);

		vkDestroyDescriptorPool(s_device, s_descriptor_pool, s_allocator);
#ifdef MZ_DEBUG
		vkDestroyDebugUtilsMessengerEXT(s_instance, s_debug_utils, s_allocator);
#endif
		vkDestroyDevice(s_device, s_allocator);
		vkDestroyInstance(s_instance, s_allocator);

		glfwDestroyWindow(window_);
		glfwTerminate();
	}

	VkInstance Application::get_instance()
	{
		return s_instance;
	}

	VkDevice Application::get_device()
	{
		return s_device;
	}

	VkPhysicalDevice Application::get_physical_device()
	{
		return s_physical_device;
	}

	VkCommandBuffer Application::get_command_buffer(bool begin)
	{
		VkCommandPool command_pool = s_window.Frames[s_window.FrameIndex].CommandPool;

		VkCommandBufferAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.commandPool = command_pool;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandBufferCount = 1;

		VkCommandBuffer& command_buffer = s_all_command_buffers[s_window.FrameIndex].emplace_back();
		VkResult err = vkAllocateCommandBuffers(s_device, &alloc_info, &command_buffer);
		check_vk_result(err);

		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags |= begin ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0;
		err = vkBeginCommandBuffer(command_buffer, &begin_info);
		check_vk_result(err);

		return command_buffer;
	}

	void Application::flush_command_buffer(VkCommandBuffer command_buffer)
	{
		constexpr uint64_t fence_timeout = 100000000;

		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer;
		VkResult err = vkEndCommandBuffer(command_buffer);
		check_vk_result(err);

		VkFenceCreateInfo fence_info = {};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = 0;
		VkFence fence;
		err = vkCreateFence(s_device, &fence_info, s_allocator, &fence);
		check_vk_result(err);

		err = vkQueueSubmit(s_queue, 1, &submit_info, fence);
		check_vk_result(err);

		err = vkWaitForFences(s_device, 1, &fence, VK_TRUE, fence_timeout);
		check_vk_result(err);

		vkDestroyFence(s_device, fence, s_allocator);
	}

	void Application::submit_resource_free(const std::function<void()>& func)
	{
		s_resource_free_queue[s_current_frame_index].emplace_back(func);
	}
}

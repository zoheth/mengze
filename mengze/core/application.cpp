#include "application.h"

#include <corecrt_io.h>
#include <functional>

#include <volk.h>
#include "GLFW/glfw3.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include "logging.h"


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


	void check_vk_result(VkResult err)
	{
		if (err == 0)
			return;
		LOGE("[vulkan] Error: VkResult = {}", err)
			if (err < 0)
				abort();
	}

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
			VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			VkSubmitInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			info.waitSemaphoreCount = 1;
			info.pWaitSemaphores = &image_acquired_semaphore;
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
	Application::Application(const ApplicationSpecification& spec)
		:spec_(spec)
	{

		init();
	}

	Application::~Application()
	{
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
}

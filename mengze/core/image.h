#pragma once

#include <string>
#include <volk.h>

namespace mengze
{
	enum class ImageFormat
	{
		kUndefined,
		kR8G8B8A8
	};

	class Image
	{
	public:
		Image(uint32_t width, uint32_t height, ImageFormat format, const void* data = nullptr);
		~Image();

		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;
		Image(Image&&) = delete;
		Image& operator=(Image&&) = delete;


		void set_data(const void* data);

		[[nodiscard]] VkDescriptorSet get_descriptor_set() const { return descriptor_set_; }

		void resize(uint32_t width, uint32_t height);

		uint32_t get_width() const { return width_; }
		uint32_t get_height() const { return height_; }

	private:
		void allocate_memory();
		void release_memory();
	private:
		uint32_t width_{ 0 };
		uint32_t height_{ 0 };
		ImageFormat format_{ ImageFormat::kUndefined };
		VkImage image_{ VK_NULL_HANDLE };
		VkDeviceMemory memory_{ VK_NULL_HANDLE };
		VkImageView image_view_{ VK_NULL_HANDLE };
		VkSampler sampler_{ VK_NULL_HANDLE };

		VkBuffer staging_buffer_{ VK_NULL_HANDLE };
		VkDeviceMemory staging_memory_{ VK_NULL_HANDLE };

		uint64_t aligned_size_{ 0 };

		VkDescriptorSet descriptor_set_{ VK_NULL_HANDLE };
	};
}

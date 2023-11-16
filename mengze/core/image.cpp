#include "image.h"

#include "application.h"
#include "backends/imgui_impl_vulkan.h"

namespace
{
	uint32_t get_vulkan_memory_type(VkMemoryPropertyFlags properties, uint32_t type_bits)
	{
		VkPhysicalDeviceMemoryProperties memory_properties;
		vkGetPhysicalDeviceMemoryProperties(mengze::Application::get_physical_device(), &memory_properties);

		for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i)
		{
			if ((type_bits & (1 << i)) &&
				((memory_properties.memoryTypes[i].propertyFlags & properties) == properties))
			{
				return i;
			}
		}

		return 0xffffffff;
	}

	uint32_t bytes_per_pixel(mengze::ImageFormat format)
	{
		switch (format)
		{
		case mengze::ImageFormat::kR8G8B8A8:
			return 4;
		default:
			return 0;
		}
	}

	VkFormat to_vk_format(mengze::ImageFormat format)
	{
		switch (format)
		{
		case mengze::ImageFormat::kR8G8B8A8:
			return VK_FORMAT_R8G8B8A8_UNORM;
		default:
			return VK_FORMAT_UNDEFINED;
		}
	}
}

namespace mengze
{
	Image::Image(uint32_t width, uint32_t height, ImageFormat format, const void* data)
		:width_(width), height_(height), format_(format)
	{
		allocate_memory();
		if (data)
			set_data(data);
	}

	Image::~Image()
	{
		release_memory();
	}

	void Image::set_data(const void* data)
	{
		VkDevice device = Application::get_device();

		size_t upload_size = width_ * height_ * bytes_per_pixel(format_);

		VkResult err;

		if (!staging_buffer_)
		{
			// Create the Upload Buffer
			{
				VkBufferCreateInfo buffer_info = {};
				buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				buffer_info.size = upload_size;
				buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
				err = vkCreateBuffer(device, &buffer_info, nullptr, &staging_buffer_);
				check_vk_result(err);
				VkMemoryRequirements req;
				vkGetBufferMemoryRequirements(device, staging_buffer_, &req);
				aligned_size_ = req.size;
				VkMemoryAllocateInfo alloc_info = {};
				alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				alloc_info.allocationSize = req.size;
				alloc_info.memoryTypeIndex = get_vulkan_memory_type(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, req.memoryTypeBits);
				err = vkAllocateMemory(device, &alloc_info, nullptr, &staging_memory_);
				check_vk_result(err);
				err = vkBindBufferMemory(device, staging_buffer_, staging_memory_, 0);
				check_vk_result(err);
			}
		}

		// Upload to Buffer
		{
			char* map = nullptr;
			err = vkMapMemory(device, staging_memory_, 0, aligned_size_, 0, (void**)&map);
			check_vk_result(err);
			memcpy(map, data, upload_size);
			VkMappedMemoryRange range[1] = {};
			range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			range[0].memory = staging_memory_;
			range[0].size = aligned_size_;
			err = vkFlushMappedMemoryRanges(device, 1, range);
			check_vk_result(err);
			vkUnmapMemory(device, staging_memory_);
		}

		// Copy to Image
		{
			VkCommandBuffer command_buffer = Application::get_command_buffer(true);

			VkImageMemoryBarrier copy_barrier = {};
			copy_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			copy_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			copy_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			copy_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			copy_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			copy_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			copy_barrier.image = image_;
			copy_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copy_barrier.subresourceRange.levelCount = 1;
			copy_barrier.subresourceRange.layerCount = 1;
			vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &copy_barrier);

			VkBufferImageCopy copy_region = {};
			copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copy_region.imageSubresource.layerCount = 1;
			copy_region.imageExtent.width = width_;
			copy_region.imageExtent.height = height_;
			copy_region.imageExtent.depth = 1;
			vkCmdCopyBufferToImage(command_buffer, staging_buffer_, image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

			VkImageMemoryBarrier use_barrier = {};
			use_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			use_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			use_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			use_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			use_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			use_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			use_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			use_barrier.image = image_;
			use_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			use_barrier.subresourceRange.levelCount = 1;
			use_barrier.subresourceRange.layerCount = 1;
			vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &use_barrier);

			Application::flush_command_buffer(command_buffer);
		}
	}

	void Image::resize(uint32_t width, uint32_t height)
	{
		if(width == width_ && height == height_)
			return;
		width_ = width;
		height_ = height;
		allocate_memory();
	}

	void Image::allocate_memory()
	{
		VkDevice device = Application::get_device();
		VkResult res;

		VkFormat vk_format = to_vk_format(format_);

		{
			VkImageCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			info.imageType = VK_IMAGE_TYPE_2D;
			info.format = vk_format;
			info.extent.width = width_;
			info.extent.height = height_;
			info.extent.depth = 1;
			info.mipLevels = 1;
			info.arrayLayers = 1;
			info.samples = VK_SAMPLE_COUNT_1_BIT;
			info.tiling = VK_IMAGE_TILING_OPTIMAL;
			info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			res = vkCreateImage(device, &info, nullptr, &image_);
			check_vk_result(res);
			VkMemoryRequirements req;
			vkGetImageMemoryRequirements(device, image_, &req);

			VkMemoryAllocateInfo alloc_info = {};
			alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			alloc_info.allocationSize = req.size;
			alloc_info.memoryTypeIndex = get_vulkan_memory_type(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, req.memoryTypeBits);
			res = vkAllocateMemory(device, &alloc_info, nullptr, &memory_);
			check_vk_result(res);
			res = vkBindImageMemory(device, image_, memory_, 0);
			check_vk_result(res);
		}

		{
			VkImageViewCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			info.image = image_;
			info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			info.format = vk_format;
			info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			info.subresourceRange.levelCount = 1;
			info.subresourceRange.layerCount = 1;
			res = vkCreateImageView(device, &info, nullptr, &image_view_);
			check_vk_result(res);
		}

		{
			VkSamplerCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			info.magFilter = VK_FILTER_LINEAR;
			info.minFilter = VK_FILTER_LINEAR;
			info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			info.mipLodBias = 0.0f;
			info.compareOp = VK_COMPARE_OP_NEVER;
			info.minLod = 0.0f;
			info.maxLod = 0.0f;
			info.maxAnisotropy = 1.0f;
			info.anisotropyEnable = VK_FALSE;
			res = vkCreateSampler(device, &info, nullptr, &sampler_);
			check_vk_result(res);
		}

		descriptor_set_ = ImGui_ImplVulkan_AddTexture(sampler_, image_view_, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void Image::release_memory()
	{
		Application::submit_resource_free(
			[sampler = sampler_, image_view = image_view_, image = image_, memory = memory_, staging_buffer = staging_buffer_, staging_memory = staging_memory_]()
			{
				VkDevice device = Application::get_device();
				vkDeviceWaitIdle(device);

				vkDestroySampler(device, sampler, nullptr);
				vkDestroyImageView(device, image_view, nullptr);
				vkDestroyImage(device, image, nullptr);
				vkFreeMemory(device, memory, nullptr);
				vkDestroyBuffer(device, staging_buffer, nullptr);
				vkFreeMemory(device, staging_memory, nullptr);
			});
		sampler_ = VK_NULL_HANDLE;
		image_view_ = VK_NULL_HANDLE;
		image_ = VK_NULL_HANDLE;
		memory_ = VK_NULL_HANDLE;
		staging_buffer_ = VK_NULL_HANDLE;
		staging_memory_ = VK_NULL_HANDLE;

	}

}

#pragma once

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include <tiny_gltf.h>

#include "scene_graph/scene.h"


namespace mengze
{
	class GltfLoader
	{
	public:
		GltfLoader();

		std::unique_ptr<Scene> read_scene_from_file(const std::string& filename, int scene_index=-1);
	private:
		std::unique_ptr<Scene> load_scene(int scene_index = -1);
	private:
		tinygltf::Model model_;
		std::string model_path_;
	};
}
#include "gltf_loader.h"

#include "core/logging.h"

namespace mengze
{
	std::unique_ptr<Scene> GltfLoader::read_scene_from_file(const std::string& filename, int scene_index)
	{
		std::string err;
		std::string warn;

		tinygltf::TinyGLTF gltf_loader;

		const std::string gltf_file = "asserts/" + filename;

		const bool import_result = gltf_loader.LoadASCIIFromFile(&model_, &err, &warn, gltf_file);

		if (!import_result)
		{
			LOGE("Failed to load gltf file {}.", gltf_file.c_str());

			return nullptr;
		}

		if (!err.empty())
		{
			LOGE("Error loading gltf model: {}.", err.c_str());

			return nullptr;
		}

		if (!warn.empty())
		{
			LOGI("{}", warn.c_str());
		}

		size_t pos = filename.find_last_of('/');

		model_path_ = filename.substr(0, pos);

		if (pos == std::string::npos)
		{
			model_path_.clear();
		}

		return std::make_unique<Scene>(load_scene(scene_index));
	}

	Scene GltfLoader::load_scene(int scene_index)
	{
		auto scene = Scene();


	}
}

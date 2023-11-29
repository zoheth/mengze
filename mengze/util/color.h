#pragma once
#include <algorithm> 
#include <string>

namespace mengze
{
	class Rgb
	{
	public:
		Rgb() = default;
		Rgb(float r, float g, float b) : r{ r }, g{ g }, b{ b } {}

		[[nodiscard]] Rgb clamp() const
		{
			return {
				std::clamp(r, 0.0f, 1.0f),
				std::clamp(g, 0.0f, 1.0f),
				std::clamp(b, 0.0f, 1.0f)};
		}

		float r{ 0 }, g{0}, b{0};
	};
}
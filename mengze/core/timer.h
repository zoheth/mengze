#pragma once

#include <chrono>

namespace mengze
{
	class Timer
	{
	public:
		Timer()
		{
			reset();
		}

		void reset()
		{
			start_ = std::chrono::high_resolution_clock::now();
		}

		float elapsed() const
		{
			return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start_).count() * 0.001f * 0.001f;
		}

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> start_;
	};
}
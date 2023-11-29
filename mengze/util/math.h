#pragma once
#include "mengze.h"

namespace mengze
{
	template <template <typename> class Child, typename T>
	class Tuple3
	{
	public:
		Tuple3() = default;

		MZ_CPU_GPU
			Tuple3(T x, T y, T z) : x(x), y(y), z(z) {}

		T x{}, y{}, z{};
	};

	template <typename T>
	class Vector3 : public Tuple3<Vector3, T>
	{
	public:
		using Tuple3<Vector3, T>::x;
		using Tuple3<Vector3, T>::y;
		using Tuple3<Vector3, T>::z;

		Vector3() = default;

		MZ_CPU_GPU
			Vector3(T x, T y, T z) : Tuple3<Vector3, T>(x, y, z) {}

	};

	template <typename T>
	class Point3 : public Tuple3<Point3, T>
	{
	public:
		using Tuple3<Point3, T>::x;
		using Tuple3<Point3, T>::y;
		using Tuple3<Point3, T>::z;

		Point3() = default;

		MZ_CPU_GPU
			Point3(T x, T y, T z) : Tuple3<Point3, T>(x, y, z) {}
	};

}
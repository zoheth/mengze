#include "ray_tracing/bvh.h"

#include <future>
#include <thread>

#include <ctpl_stl.h>

#include "scene.h"

namespace 
{
	ctpl::thread_pool s_thread_pool(8);
}

namespace mengze::rt
{
BvhNode::BvhNode(const HittableList &list) :
    BvhNode(list.objects(), 0, list.objects().size())
{}

BvhNode::BvhNode(const std::vector<std::shared_ptr<Hittable>> &src_objects, size_t start, size_t end, bool is_root, int depth)
{
	auto objects = src_objects;
	if (is_root)
	{
		is_root_          = true;
		root_all_objects_ = src_objects;
	}

	auto axis       = static_cast<int>(3 * random_float());
	auto comparator = (axis == 0) ? box_x_compare : (axis == 1) ? box_y_compare :
	                                                              box_z_compare;

	auto object_span = end - start;

	if (object_span == 1)
	{
		left_ = right_ = objects[start];
	}
	else if (object_span == 2)
	{
		if (comparator(objects[start], objects[start + 1]))
		{
			left_  = objects[start];
			right_ = objects[start + 1];
		}
		else
		{
			left_  = objects[start + 1];
			right_ = objects[start];
		}
	}
	else
	{
		std::sort(objects.begin() + start, objects.begin() + end, comparator);

		auto mid = start + object_span / 2;

		if (depth == 3)
		{
			/*auto left_future = std::async(std::launch::async, [&objects, start, mid]() {
				return std::make_shared<BvhNode>(objects, start, mid, false);
			});

			auto right_future = std::async(std::launch::async, [&objects, mid, end]() {
				return std::make_shared<BvhNode>(objects, mid, end, false);
			});*/

			auto left_future = s_thread_pool.push([=, &objects](int id) {
				return std::make_shared<BvhNode>(objects, start, mid, false, depth+1);
			});

			auto right_future = s_thread_pool.push([=, &objects](int id) {
				return std::make_shared<BvhNode>(objects, mid, end, false, depth+1);
			});

			left_  = left_future.get();
			right_ = right_future.get();
		}
		else
		{
			left_  = std::make_shared<BvhNode>(objects, start, mid, false);
			right_ = std::make_shared<BvhNode>(objects, mid, end, false);
		}
	}

	auto box_left  = left_->bounding_box();
	auto box_right = right_->bounding_box();

	box_ = Aabb(box_left, box_right);
}

bool BvhNode::hit(const Ray &r, Interval ray_t, HitRecord &rec) const
{
	if (!box_.hit(r, ray_t))
	{
		return false;
	}

	auto hit_left  = left_->hit(r, ray_t, rec);
	auto hit_right = right_->hit(r, Interval(ray_t.min(), hit_left ? rec.t : ray_t.max()), rec);

	return hit_left || hit_right;
}

Aabb BvhNode::bounding_box() const
{
	return box_;
}

float BvhNode::pdf_value(const glm::vec3 &origin, const glm::vec3 &direction) const
{
	auto weight = 1.0f / root_all_objects_.size();
	auto sum    = 0.0f;

	for (const auto &object : root_all_objects_)
	{
		sum += weight * object->pdf_value(origin, direction);
	}
	return sum;
}

glm::vec3 BvhNode::random(const glm::vec3 &origin) const
{
	auto index = static_cast<int>(random_float() * root_all_objects_.size());
	index      = std::min(index, static_cast<int>(root_all_objects_.size()) - 1);
	return root_all_objects_[index]->random(origin);
}

bool BvhNode::box_compare(const std::shared_ptr<Hittable> &a, const std::shared_ptr<Hittable> &b, int axis)
{
	auto box_a = a->bounding_box();
	auto box_b = b->bounding_box();

	return box_a.axis(axis).min() < box_b.axis(axis).min();
}

bool BvhNode::box_x_compare(const std::shared_ptr<Hittable> &a, const std::shared_ptr<Hittable> &b)
{
	return box_compare(a, b, 0);
}

bool BvhNode::box_y_compare(const std::shared_ptr<Hittable> &a, const std::shared_ptr<Hittable> &b)
{
	return box_compare(a, b, 1);
}

bool BvhNode::box_z_compare(const std::shared_ptr<Hittable> &a, const std::shared_ptr<Hittable> &b)
{
	return box_compare(a, b, 2);
}
}        // namespace mengze::rt

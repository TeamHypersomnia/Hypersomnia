#pragma once
#include "augs/math/vec2.h"
#include "augs/math/transform.h"

#include "augs/misc/constant_size_vector.h"

template <
	class T,
	std::size_t vertex_count,
	std::size_t partition_index_count
>
struct basic_convex_partitioned_shape {
	using this_type = basic_convex_partitioned_shape<T, vertex_count, partition_index_count>;
	using vec2 = basic_vec2<T>;
	using transform = basic_transform<T>;
	using index_type = unsigned char;

	using original_poly_type = augs::constant_size_vector<vec2, vertex_count>;
	using convex_partition_type = augs::constant_size_vector<unsigned char, partition_index_count>;

	// GEN INTROSPECTOR struct basic_convex_partitioned_shape class T std::size_t vertex_count std::size_t partition_index_count
	original_poly_type original_poly = {};
	convex_partition_type convex_partition = {};
	// END GEN INTROSPECTOR

	void offset_vertices(const transformr transform) {
		for (auto& v : original_poly) {
			v.rotate(transform.rotation);
			v += transform.pos;
		}
	}

	void scale(const vec2 mult) {
		for (auto& v : original_poly) {
			v *= mult;
		}
	}

	void clear() {
		original_poly.clear();
		convex_partition.clear();
	}

	bool empty() const {
		return original_poly.empty();
	}

	bool take_vertices_one_after_another() const {
		return convex_partition.empty();
	}
};
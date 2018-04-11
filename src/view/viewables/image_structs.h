#pragma once
#include <optional>

#include "augs/misc/enum/enum_array.h"
#include "augs/drawing/flip.h"
#include "augs/texture_atlas/texture_atlas_entry.h"
#include "game/assets/ids/asset_ids.h"
#include "view/viewables/all_viewables_declarations.h"
#include "game/components/shape_polygon_component.h"
#include "game/components/sprite_component.h"
#include "game/components/polygon_component.h"

struct image_usage_as_button {
	// GEN INTROSPECTOR struct image_usage_as_button
	flip_flags flip;
	vec2 bbox_expander;
	// END GEN INTROSPECTOR
};

struct image_meta {
	// GEN INTROSPECTOR struct image_meta
	image_usage_as_button usage_as_button;
	// END GEN INTROSPECTOR
};

struct image_cache {
	image_cache(
		const image_loadables_def&,
		const image_meta&
	);

	vec2u original_image_size;
	convex_partitioned_shape partitioned_shape;

	vec2u get_size() const {
		return original_image_size;
	}
};

struct image_in_atlas {
	augs::texture_atlas_entry diffuse;
	augs::texture_atlas_entry neon_map;
	augs::texture_atlas_entry desaturated;

	operator augs::texture_atlas_entry() const {
		return diffuse;
	}

	vec2u get_size() const {
		return diffuse.get_original_size();
	}
};

struct loaded_image_caches_map : public std::vector<image_cache> {
	using base = std::vector<image_cache>;
	using base::base;

	loaded_image_caches_map() = default;

	explicit loaded_image_caches_map(
		const image_loadables_map&,
		const image_metas_map&
	);

	decltype(auto) operator[](const assets::image_id id) {
		return base::operator[](id.indirection_index);
	}

	decltype(auto) operator[](const assets::image_id id) const {
		return base::operator[](id.indirection_index);
	}
};

template <class E>
void add_shape_invariant_from_renderable(
	E& into,
	const loaded_image_caches_map& caches
) {
	static_assert(E::template has<invariants::shape_polygon>());

	if (const auto sprite = into.template find<invariants::sprite>()) {
		const auto image_size = caches.at(sprite->tex).get_size();
		vec2 scale = sprite->get_size() / image_size;

		invariants::shape_polygon shape_polygon_def;

		shape_polygon_def.shape = caches.at(sprite->tex).partitioned_shape;
		shape_polygon_def.shape.scale(scale);

		into.template set(shape_polygon_def);
	}

	if (const auto polygon = into.template find<invariants::polygon>()) {
		std::vector<vec2> input;

		input.reserve(polygon->vertices.size());

		for (const auto& v : polygon->vertices) {
			input.push_back(v.pos);
		}

		invariants::shape_polygon shape_polygon_def;
		shape_polygon_def.shape.add_concave_polygon(input);

		into.template set(shape_polygon_def);
	}
}

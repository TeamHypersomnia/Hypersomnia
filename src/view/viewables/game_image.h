#pragma once
#include <optional>

#include "augs/misc/enum/enum_array.h"
#include "augs/drawing/flip.h"
#include "augs/texture_atlas/texture_atlas_entry.h"
#include "game/assets/ids/game_image_id.h"
#include "view/viewables/all_viewables_declarations.h"
#include "game/components/shape_polygon_component.h"

struct game_image_usage_as_button {
	// GEN INTROSPECTOR struct game_image_usage_as_button
	flip_flags flip;
	vec2 bbox_expander;
	// END GEN INTROSPECTOR
};

struct game_image_meta {
	// GEN INTROSPECTOR struct game_image_meta
	game_image_usage_as_button usage_as_button;
	// END GEN INTROSPECTOR
};

struct game_image_cache {
	game_image_cache(
		const game_image_loadables&,
		const game_image_meta&
	);

	vec2u original_image_size;
	convex_partitioned_shape partitioned_shape;

	vec2u get_size() const {
		return original_image_size;
	}
};

struct game_image_in_atlas {
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

struct loaded_game_image_caches : public asset_map<
	assets::game_image_id,
	game_image_cache
> {
	loaded_game_image_caches() = default;

	explicit loaded_game_image_caches(
		const game_image_loadables_map&,
		const game_image_metas_map&
	);
};

class entity_flavour;

void add_shape_invariant_from_renderable(
	entity_flavour& into,
	const loaded_game_image_caches& caches
);

#pragma once
#include "application/setups/editor/resources/editor_pathed_resource.h"
#include "view/viewables/ad_hoc_in_atlas_map.h"

#include "augs/math/vec2.h"
#include "augs/drawing/sprite.h"
#include "game/assets/ids/asset_ids.h"
#include "game/cosmos/entity_flavour_id.h"
#include "view/viewables/regeneration/neon_maps.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"

#include "test_scenes/test_scene_flavour_ids.h"
#include "augs/misc/convex_partitioned_shape.h"
#include "game/assets/image_offsets.h"
#include "application/setups/editor/resources/editor_sound_effect.h"

enum class editor_sprite_domain {
	// GEN INTROSPECTOR enum class editor_sprite_domain
	BACKGROUND,
	PHYSICAL,
	FOREGROUND,

	COUNT
	// END GEN INTROSPECTOR
};

struct editor_material_resource;

struct editor_custom_footstep {
	// GEN INTROSPECTOR struct editor_custom_footstep
	editor_sound_effect sound;
	float walking_speed = 1.0f;
	// END GEN INTROSPECTOR

	bool operator==(const editor_custom_footstep&) const = default;
};

struct editor_sprite_resource_nonphysical {
	// GEN INTROSPECTOR struct editor_sprite_resource_nonphysical
	bool cover_background_neons = false;
	bool illuminate_like_wall = false;
	bool full_illumination = false;

	augs::maybe<editor_custom_footstep> custom_footstep;
	// END GEN INTROSPECTOR

	bool operator==(const editor_sprite_resource_nonphysical&) const = default;
};

struct editor_sprite_resource_physical {
	// GEN INTROSPECTOR struct editor_sprite_resource_physical
	bool is_static = false;
	bool is_walk_through = false;
	bool is_see_through = false;
	bool is_throw_through = false;
	bool is_melee_throw_through = false;
	bool is_shoot_through = false;

	real32 density = 0.7f;
	real32 friction = 0.0f;
	real32 bounciness = 0.2f;
	real32 penetrability = 1.0f;

	real32 linear_damping = 6.5f;
	real32 angular_damping = 6.5f;

	real32 collision_sound_sensitivity = 1.0f;

	editor_typed_resource_id<editor_material_resource> material;

	bool cover_background_neons = true;
	bool illuminate_like_wall = true;

	image_shape_type custom_shape;
	// END GEN INTROSPECTOR

	bool operator==(const editor_sprite_resource_physical&) const = default;
};

struct editor_sprite_resource_editable {
	// GEN INTROSPECTOR struct editor_sprite_resource_editable
	editor_sprite_domain domain = editor_sprite_domain::BACKGROUND;
	augs::maybe<neon_map_input> neon_map;

	rgba color = white;
	rgba neon_color = white;

	vec2i size = vec2i::zero;
	bool stretch_when_resized = false;

	augs::maybe<float> color_wave_speed = augs::maybe<float>(1.0f, false);
	augs::maybe<float> rotate_continuously_degrees_per_sec = augs::maybe<float>(360.0f, false);

	augs::maybe<intensity_vibration_input> neon_alpha_vibration;
	bool vibrate_diffuse_too = false;

	editor_sprite_resource_physical as_physical;
	editor_sprite_resource_nonphysical as_nonphysical;
	// END GEN INTROSPECTOR

	bool operator==(const editor_sprite_resource_editable&) const = default;
};

struct editor_sprite_node;
struct editor_sprite_resource {
	using node_type = editor_sprite_node;

	std::vector<int> animation_frames;

	editor_pathed_resource external_file;
	editor_sprite_resource_editable editable;

	/* Cache */

	ad_hoc_entry_id thumbnail_id = static_cast<ad_hoc_entry_id>(-1);

	std::optional<std::variant<
		test_static_decorations,
		test_plain_sprited_bodies,
		test_dynamic_decorations
	>> official_tag;

	std::string cached_official_name;

	/* Only for quick mapping */
	mutable std::variant<
		typed_entity_flavour_id<static_decoration>,
		typed_entity_flavour_id<plain_sprited_body>,
		typed_entity_flavour_id<dynamic_decoration>
	> scene_flavour_id;

	mutable assets::image_id scene_asset_id;
	mutable assets::plain_animation_id scene_animation_id;

	mutable uint32_t reference_count = 0u;
	mutable bool changes_detected = false;

	mutable std::string resolved_pseudoid;
	mutable bool found_on_disk = false;

	bool should_be_tracked() const {
		return reference_count > 0 || changes_detected;
	}

	bool unbacked_on_disk() const {
		return !found_on_disk;
	}

	editor_sprite_resource(const editor_pathed_resource& f) : external_file(f) {}

	decltype(auto) get_display_name() const {
		if (!cached_official_name.empty()) {
			return cached_official_name;
		}

		return external_file.get_display_name();
	}

	static const char* get_type_name() {
		return "Sprite";
	}
};

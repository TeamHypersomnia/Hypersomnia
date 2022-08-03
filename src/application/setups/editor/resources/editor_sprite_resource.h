#pragma once
#include "application/setups/editor/resources/editor_pathed_resource.h"
#include "view/viewables/ad_hoc_in_atlas_map.h"

#include "augs/math/vec2.h"
#include "augs/drawing/sprite.h"
#include "game/assets/ids/asset_ids.h"
#include "game/cosmos/entity_flavour_id.h"

struct editor_sprite_resource_editable {
	// GEN INTROSPECTOR struct editor_sprite_resource_editable
	rgba color = white;
	vec2i size = vec2i::zero;
	bool stretch_when_resized = false;
	// END GEN INTROSPECTOR
};

struct editor_sprite_node;
struct editor_sprite_resource {
	using node_type = editor_sprite_node;

	editor_pathed_resource external_file;
	editor_sprite_resource_editable editable;

	/* Cache */

	ad_hoc_entry_id thumbnail_id = static_cast<ad_hoc_entry_id>(-1);

	/* Only for quick mapping */
	mutable entity_flavour_id scene_flavour_id;

	editor_sprite_resource(const editor_pathed_resource& f) : external_file(f) {}

	decltype(auto) get_display_name() const {
		return external_file.get_display_name();
	}
};

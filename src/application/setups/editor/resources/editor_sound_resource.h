#pragma once
#include "application/setups/editor/resources/editor_pathed_resource.h"

#include "game/detail/view_input/sound_effect_modifier.h"

struct editor_sound_resource_editable : sound_effect_modifier {
	using base = sound_effect_modifier;
	using introspect_base = base;
};

struct editor_sound_node;
struct editor_sound_resource {
	using node_type = editor_sound_node;

	// GEN INTROSPECTOR struct editor_sound_resource
	editor_pathed_resource external_file;
	editor_sound_resource_editable editable;
	// END GEN INTROSPECTOR

	/* Only for quick mapping */
	mutable std::variant<
		typed_entity_flavour_id<sound_decoration>
	> scene_flavour_id;

	editor_sound_resource(const editor_pathed_resource& f) : external_file(f) {}

	decltype(auto) get_display_name() const {
		return external_file.get_display_name();
	}
};

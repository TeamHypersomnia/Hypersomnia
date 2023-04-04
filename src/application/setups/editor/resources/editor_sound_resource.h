#pragma once
#include "application/setups/editor/resources/editor_pathed_resource.h"
#include "application/setups/editor/resources/editor_sound_effect.h"

struct editor_sound_resource_editable : editor_sound_effect_modifier {
	using base = editor_sound_effect_modifier;
	using introspect_base = base;
};

struct editor_sound_node;
struct editor_sound_resource {
	using node_type = editor_sound_node;

	editor_pathed_resource external_file;
	editor_sound_resource_editable editable;

	std::optional<std::variant<test_sound_decorations>> official_tag;

	std::string cached_official_name;

	/* Only for quick mapping */
	mutable std::variant<
		typed_entity_flavour_id<sound_decoration>
	> scene_flavour_id;

	mutable assets::sound_id scene_asset_id;

	mutable uint32_t reference_count = 0u;
	mutable bool changes_detected = false;

	bool should_be_tracked() const {
		return reference_count > 0 || changes_detected;
	}

	mutable std::string resolved_pseudoid;
	mutable std::optional<bool> missing_on_disk;

	editor_sound_resource(const editor_pathed_resource& f) : external_file(f) {}

	decltype(auto) get_display_name() const {
		if (!cached_official_name.empty()) {
			return cached_official_name;
		}

		return external_file.get_display_name();
	}

	static const char* get_type_name() {
		return "Sound";
	}
};

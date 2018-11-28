#pragma once
#include <unordered_set>

#include "augs/graphics/imgui_payload.h"

#include "application/setups/editor/detail/editor_image_preview.h"

#include "application/setups/editor/editor_command_input.h"
#include "augs/misc/imgui/standard_window_mixin.h"
#include "application/setups/editor/gui/asset_path_browser_settings.h"
#include "application/setups/editor/property_editor/property_editor_structs.h"

class images_in_atlas_map;

namespace augs {
	class window;
	struct introspection_access;
}

template <class asset_id_type>
struct separate_properties_window {
	// GEN INTROSPECTOR struct separate_properties_window class asset_id_type
	bool show = true;
	asset_id_type currently_viewed;
	// END GEN INTROSPECTOR
};

template <class asset_id_type>
struct editor_pathed_asset_gui : standard_window_mixin<editor_pathed_asset_gui<asset_id_type>> {
	using base = standard_window_mixin<editor_pathed_asset_gui<asset_id_type>>;
	using base::base;

	using introspect_base = base;

	friend augs::introspection_access;

	// GEN INTROSPECTOR struct editor_pathed_asset_gui class asset_id_type
	asset_path_browser_settings path_browser_settings;
private:
	separate_properties_window<asset_id_type> separate_properties;
public:
	// END GEN INTROSPECTOR

	bool acquire_missing_paths = true;
	int move_currently_viewed_by = 0;

	void perform(
		const augs::window&,
		const property_editor_settings&, 
		const images_in_atlas_map&, 
		editor_command_input
	);

	bool is_separate_properties_focused() const {
		return base::show && separate_properties_focused;
	}

private:
	std::unordered_set<asset_id_type> ticked_assets;
	property_editor_state property_editor_data;
	bool separate_properties_focused = false;

	editor_image_preview preview;

	void set_currently_viewed(asset_id_type);
};

using editor_images_gui = editor_pathed_asset_gui<assets::image_id>;
using editor_sounds_gui = editor_pathed_asset_gui<assets::sound_id>;

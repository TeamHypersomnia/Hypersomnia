#pragma once
#include <unordered_set>

#include "augs/graphics/imgui_payload.h"

#include "application/setups/debugger/detail/debugger_image_preview.h"

#include "application/setups/debugger/debugger_command_input.h"
#include "augs/misc/imgui/standard_window_mixin.h"
#include "application/setups/debugger/gui/asset_path_browser_settings.h"
#include "application/setups/debugger/property_debugger/property_debugger_structs.h"

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
struct debugger_pathed_asset_gui : standard_window_mixin<debugger_pathed_asset_gui<asset_id_type>> {
	using base = standard_window_mixin<debugger_pathed_asset_gui<asset_id_type>>;
	using base::base;

	using introspect_base = base;

	friend augs::introspection_access;

	// GEN INTROSPECTOR struct debugger_pathed_asset_gui class asset_id_type
	asset_path_browser_settings path_browser_settings;
private:
	separate_properties_window<asset_id_type> separate_properties;
public:
	// END GEN INTROSPECTOR

	bool acquire_missing_paths = true;
	int move_currently_viewed_by = 0;

	void perform(
		augs::window&,
		const property_debugger_settings&, 
		const images_in_atlas_map&, 
		debugger_command_input
	);

	bool is_separate_properties_focused() const {
		return base::show && separate_properties_focused;
	}

private:
	std::unordered_set<asset_id_type> ticked_assets;
	property_debugger_state property_debugger_data;
	bool separate_properties_focused = false;

	debugger_image_preview preview;

	void set_currently_viewed(asset_id_type);
};

using debugger_images_gui = debugger_pathed_asset_gui<assets::image_id>;
using debugger_sounds_gui = debugger_pathed_asset_gui<assets::sound_id>;

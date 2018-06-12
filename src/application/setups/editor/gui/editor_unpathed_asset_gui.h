#pragma once
#include <unordered_set>

#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/gui/standard_window_mixin.h"
#include "application/setups/editor/property_editor/property_editor_structs.h"

class images_in_atlas_map;

template <class asset_id_type>
struct editor_unpathed_asset_gui : standard_window_mixin<editor_unpathed_asset_gui<asset_id_type>> {
	using base = standard_window_mixin<editor_unpathed_asset_gui<asset_id_type>>;
	using base::base;

	using introspect_base = base;

	// GEN INTROSPECTOR struct editor_unpathed_asset_gui class asset_id_type
	bool show_orphaned = true;
	bool show_using_locations = false;
	bool preview_animations = true;
	// END GEN INTROSPECTOR

	bool acquire_missing_paths = true;

	void perform(
		const property_editor_settings&, 
		const images_in_atlas_map&,
		editor_command_input
	);

private:
	std::unordered_set<asset_id_type> ticked_assets;

	property_editor_state property_editor_data;
};

using editor_plain_animations_gui = editor_unpathed_asset_gui<assets::plain_animation_id>;
using editor_particle_effects_gui = editor_unpathed_asset_gui<assets::particle_effect_id>;

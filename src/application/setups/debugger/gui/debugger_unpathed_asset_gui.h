#pragma once
#include <unordered_set>

#include "application/setups/debugger/debugger_command_input.h"
#include "augs/misc/imgui/standard_window_mixin.h"
#include "application/setups/debugger/property_debugger/property_debugger_structs.h"
#include "game/assets/ids/asset_ids.h"

class images_in_atlas_map;

template <class asset_id_type>
struct debugger_unpathed_asset_gui : standard_window_mixin<debugger_unpathed_asset_gui<asset_id_type>> {
	using base = standard_window_mixin<debugger_unpathed_asset_gui<asset_id_type>>;
	using base::base;

	using introspect_base = base;

	// GEN INTROSPECTOR struct debugger_unpathed_asset_gui class asset_id_type
	bool show_orphaned = true;
	bool show_using_locations = false;
	bool preview_animations = true;
	// END GEN INTROSPECTOR

	bool acquire_missing_paths = true;

	void perform(
		const property_debugger_settings&, 
		const images_in_atlas_map&,
		debugger_command_input
	);

private:
	std::unordered_set<asset_id_type> ticked_assets;

	property_debugger_state property_debugger_data;
};

using debugger_plain_animations_gui = debugger_unpathed_asset_gui<assets::plain_animation_id>;
using debugger_particle_effects_gui = debugger_unpathed_asset_gui<assets::particle_effect_id>;

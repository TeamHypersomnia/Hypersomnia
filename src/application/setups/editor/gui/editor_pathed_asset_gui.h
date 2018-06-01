#pragma once
#include <unordered_set>

#include "augs/image/image.h"
#include "view/maybe_official_path.h"
#include "augs/graphics/imgui_payload.h"

#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/gui/standard_window_mixin.h"
#include "application/setups/editor/gui/asset_path_browser_settings.h"
#include "application/setups/editor/property_editor/property_editor_structs.h"

struct editor_image_preview {
	augs::image image;
	maybe_official_path<assets::image_id> path;
};

class images_in_atlas_map;

template <class asset_id_type>
struct editor_pathed_asset_gui : standard_window_mixin<editor_pathed_asset_gui<asset_id_type>> {
	using base = standard_window_mixin<editor_pathed_asset_gui<asset_id_type>>;
	using base::base;

	using introspect_base = base;

	// GEN INTROSPECTOR struct editor_pathed_asset_gui class asset_id_type
	asset_path_browser_settings path_browser_settings;
	// END GEN INTROSPECTOR

	bool acquire_missing_paths = true;

	void perform(const property_editor_settings&, const images_in_atlas_map&, editor_command_input);

private:
	std::unordered_set<asset_id_type> ticked_assets;
	property_editor_state property_editor_data;

	editor_image_preview preview;
};

using editor_images_gui = editor_pathed_asset_gui<assets::image_id>;
using editor_sounds_gui = editor_pathed_asset_gui<assets::sound_id>;

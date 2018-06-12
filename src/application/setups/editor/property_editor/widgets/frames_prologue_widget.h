#pragma once
#include "augs/filesystem/path.h"
#include "game/assets/all_logical_assets.h"
#include "view/viewables/all_viewables_defs.h"
#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/property_editor/property_editor_structs.h"

struct frames_prologue_widget {
	using animation_id_type = assets::plain_animation_id;

	const property_editor_input prop_in;
	const editor_command_input cmd_in;
	const animation_id_type id;

	const augs::path_type& project_dir;

	const std::vector<assets::plain_animation_id>& ticked_ids;
	const bool is_current_ticked;

	const images_in_atlas_map& game_atlas;
	const bool preview_animations;

	template <class T>
	static constexpr bool handles = false;

	template <class T>
	static constexpr bool handles_prologue = is_one_of_v<T, plain_animation_frames_type>;

	bool handle_prologue(const std::string&, plain_animation_frames_type&) const;
};


#pragma once
#include "augs/math/vec2.h"
#include "augs/gui/appearance_detector.h"
#include "augs/gui/button_corners.h"

#include "view/game_gui/game_gui_context.h"

using button_corners_info = basic_button_corners_info<assets::necessary_image_id>;
using last_assigned_type = std::variant<entity_id, entity_flavour_id>;

class hotbar_button : public game_gui_rect_node {
public:
	using base = game_gui_rect_node;
	using gui_entropy = base::gui_entropy;

	using this_in_item = dereferenced_location<hotbar_button_in_character_gui>;
	using const_this_in_item = const_dereferenced_location<hotbar_button_in_character_gui>;

	last_assigned_type last_assigned;

	augs::gui::appearance_detector detector;

	float elapsed_hover_time_ms = 0.f;

	float hover_highlight_maximum_distance = 8.f;
	float hover_highlight_duration_ms = 400.f;

	vec2i get_bbox(
		const necessary_images_in_atlas_map& necessarys,
		const image_definitions_map& defs,
		const const_entity_handle owner_transfer_capability
	) const;

	button_corners_info get_button_corners_info() const;

	void clear_assigned();

	bool is_assigned(const_entity_handle) const;
	const_entity_handle get_assigned_entity(
		const const_entity_handle owner_transfer_capability,
		int* out_stackable_count = nullptr,
		int offset = 0
	) const;

	bool is_selected_as_primary(const const_entity_handle owner_transfer_capability) const;
	bool is_selected_as_secondary(const const_entity_handle owner_transfer_capability) const;

	static void draw(const viewing_game_gui_context, const const_this_in_item this_id);

	static void respond_to_events(const game_gui_context, const this_in_item this_id, const gui_entropy& entropies);
	static void advance_elements(const game_gui_context, const this_in_item this_id, const augs::delta);
	static void rebuild_layouts(const game_gui_context, const this_in_item this_id);
};
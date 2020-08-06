#pragma once
#include <vector>
#include "augs/gui/formatted_string.h"
#include "game/cosmos/step_declaration.h"
#include "augs/graphics/renderer.h"
#include "augs/misc/timing/delta.h"

struct hud_message_settings;

struct hud_message {
	augs::gui::text::formatted_string text;

	mutable std::optional<double> first_appeared;
	mutable float spatial_index = -1;
};

struct faction_view_settings;

struct hud_messages_gui {
	std::vector<hud_message> messages;

	void standard_post_solve(
		const const_logic_step,
		const faction_view_settings&,
		const hud_message_settings& hud_settings
	);

	void advance(const hud_message_settings&);

	void draw(
		augs::renderer& renderer,
		const augs::atlas_entry blank_tex,
		const augs::baked_font& font,
		const vec2i screen_size,
		const hud_message_settings&,
		const augs::delta& frame_delta
	) const;
};


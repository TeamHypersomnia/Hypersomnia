#include "action_button.h"

void action_button::draw(
	const viewing_game_gui_context context,
	const const_this_in_item this_id,
	draw_info in
) {

}

void action_button::advance_elements(
	const game_gui_context context,
	const this_in_item this_id,
	const augs::delta dt
) {
	base::advance_elements(context, this_id, dt);

	if (this_id->detector.is_hovered) {
		this_id->elapsed_hover_time_ms += dt.in_milliseconds();
	}

	if (this_id->detector.current_appearance == augs::gui::appearance_detector::appearance::pushed) {
		this_id->elapsed_hover_time_ms = this_id->hover_highlight_duration_ms;
	}
}

void action_button::respond_to_events(
	const game_gui_context context,
	const this_in_item this_id,
	const gui_entropy& entropies
) {
	base::respond_to_events(context, this_id, entropies);

	const auto& rect_world = context.get_rect_world();
	auto& gui = context.get_character_gui();

	for (const auto& info : entropies.get_events_for(this_id)) {
		this_id->detector.update_appearance(info);

		if (info.msg == gui_event::lclick) {
		}

		if (info.msg == gui_event::hover) {
			this_id->elapsed_hover_time_ms = 0.f;
		}

		if (info.msg == gui_event::lfinisheddrag) {
		}
	}
}

void action_button::rebuild_layouts(
	const game_gui_context context,
	const this_in_item this_id
) {
	base::rebuild_layouts(context, this_id);
}
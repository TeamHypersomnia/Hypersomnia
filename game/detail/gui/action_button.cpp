#include "action_button.h"
#include "augs/gui/text_drawer.h"
#include "augs/gui/stroke.h"
#include "game/detail/gui/game_gui_context.h"
#include "game/transcendental/cosmos.h"
#include "game/systems_audiovisual/gui_element_system.h"
#include "game/components/sentience_component.h"

#include "augs/graphics/drawers.h"

void action_button::draw(
	const viewing_game_gui_context context,
	const const_this_in_item this_id,
	draw_info info
) {
	const auto intent_for_this = static_cast<intent_type>(static_cast<int>(intent_type::SPECIAL_ACTION_BUTTON_1) + this_id.get_location().index);
	const auto bound_key = context.input_information.get_bound_key_if_any(intent_for_this);

	const auto& cosmos = context.get_cosmos();
	
	const auto now = cosmos.get_timestamp();
	const auto dt = cosmos.get_fixed_delta();

	const auto absolute_rect = context.get_tree_entry(this_id).get_absolute_rect();

	if (bound_key != augs::window::event::keys::key::INVALID) {
		const auto& sentience = context.get_gui_element_entity().get<components::sentience>();
		const auto bound_spell = this_id->bound_spell;

		if (bound_spell != spell_type::COUNT && sentience.spells.find(bound_spell) != sentience.spells.end()) {
			rgba inside_col = white;

			inside_col.a = 220;

			const bool is_pushed = this_id->detector.current_appearance == augs::gui::appearance_detector::appearance::pushed;

			if (this_id->detector.is_hovered) {
				inside_col.a = 255;
			}

			assets::texture_id inside_tex = assets::texture_id::INVALID;
			assets::texture_id border_tex = assets::texture_id::SPELL_BORDER;

			rgba border_col;

			const auto appearance = get_spell_appearance(bound_spell);
			inside_tex = appearance.icon;
			border_col = appearance.border_col;

			if (inside_tex != assets::texture_id::INVALID) {
				ensure(border_tex != assets::texture_id::INVALID);

				const augs::gui::material inside_mat(inside_tex, inside_col);

				const auto absolute_icon_rect = ltrbi(vec2i(0, 0), (*inside_tex).get_size()).place_in_center_of(absolute_rect);

				draw_clipped_rect(
					inside_mat, 
					absolute_icon_rect, 
					context, 
					context.get_tree_entry(this_id).get_parent(), 
					info.v
				);

				bool is_still_cooled_down = false;

				{
					const auto all_cooldown = sentience.all_spells_cast_cooldown;
					const auto this_cooldown = sentience.spells[bound_spell].cast_cooldown;

					const auto effective_cooldown_ratio =
						all_cooldown.get_remaining_time_ms(now, dt) > this_cooldown.get_remaining_time_ms(now, dt)
						?
						all_cooldown.get_ratio_of_remaining_time(now, dt) : this_cooldown.get_ratio_of_remaining_time(now, dt);

					if (effective_cooldown_ratio > 0.f) {
						augs::draw_rectangle_clock(info.v, effective_cooldown_ratio, absolute_icon_rect, { 0, 0, 0, 200 });
						is_still_cooled_down = true;
					}
				}
				
				if (is_still_cooled_down) {
					border_col.a -= 150;
				}

				if (this_id->detector.is_hovered) {
					border_col = rgba(180, 180, 180, 255);
				}

				if (is_pushed) {
					border_col = rgba(220, 220, 220, 255);
				}

				const augs::gui::material border_mat(border_tex, border_col);

				draw_centered_texture(context, this_id, info, border_mat);

				auto label_col = cyan;
				label_col.a = 255;
				const auto label_style = augs::gui::text::style(assets::font_id::GUI_FONT, label_col );

				augs::gui::text_drawer bound_key_caption;

				bound_key_caption.set_text(
					augs::gui::text::format(
						get_key_wstring(bound_key),
						label_style
					)
				);

				bound_key_caption.bottom_right(absolute_rect);
				bound_key_caption.pos.x -= 3;
				bound_key_caption.draw_stroke(info.v);
				bound_key_caption.draw(info.v);
			}
		}
		else {
			rgba inside_col, border_col;

			inside_col = cyan;

			border_col = inside_col;
			inside_col.a = 4 * 5;
			border_col.a = 220;

			if (this_id->detector.is_hovered || this_id->detector.current_appearance == augs::gui::appearance_detector::appearance::pushed) {
				inside_col.a = 12 * 5;
				border_col.a = 255;
			}

			const auto label_style = augs::gui::text::style(assets::font_id::GUI_FONT, border_col);

			const auto inside_tex = assets::texture_id::ACTION_BUTTON_FILLED;
			const auto border_tex = assets::texture_id::ACTION_BUTTON_BORDER;

			const augs::gui::material inside_mat(inside_tex, inside_col);
			const augs::gui::material border_mat(border_tex, border_col);

			draw_centered_texture(context, this_id, info, inside_mat);
			draw_centered_texture(context, this_id, info, border_mat);

			augs::gui::text_drawer bound_key_caption;

			bound_key_caption.set_text(
				augs::gui::text::format(
					get_key_wstring(bound_key),
					label_style
				)
			);

			bound_key_caption.center(absolute_rect);
			bound_key_caption.draw_stroke(info.v);
			bound_key_caption.draw(info.v);
		}
	}
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

	for (const auto& info : entropies.get_events_for(this_id)) {
		this_id->detector.update_appearance(info);

		if (info.msg == gui_event::lclick) {
			context.get_gui_element_system().spell_requests[context.get_gui_element_entity()] = this_id->bound_spell;
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
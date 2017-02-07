#include "action_button.h"
#include "augs/gui/text_drawer.h"
#include "augs/gui/stroke.h"
#include "game/detail/gui/game_gui_context.h"
#include "game/transcendental/cosmos.h"

void action_button::draw(
	const viewing_game_gui_context context,
	const const_this_in_item this_id,
	draw_info info
) {
	const auto intent_for_this = static_cast<intent_type>(static_cast<int>(intent_type::SPECIAL_ACTION_BUTTON_1) + this_id.get_location().index);
	const auto bound_key = context.input_information.get_bound_key_if_any(intent_for_this);

	if (bound_key != augs::window::event::keys::key::INVALID) {
		const auto bound_spell = this_id->bound_spell;

		if (bound_spell != spell_type::COUNT) {
			rgba inside_col, border_col;

			inside_col = cyan;

			border_col = gray4;
			inside_col.a = 220;
			border_col.a = 220;

			if (this_id->detector.is_hovered || this_id->detector.current_appearance == augs::gui::appearance_detector::appearance::pushed) {
				inside_col.a = 255;
				border_col.a = 255;
			}

			assets::texture_id inside_tex = assets::texture_id::INVALID;
			assets::texture_id border_tex = assets::texture_id::SPELL_BORDER;
			
			const rgba blue_spell_border = { 0, 68, 179, 255 };
			const rgba green_spell_color = { 0, 200, 0, 255 };

			switch (bound_spell) {
			case spell_type::HASTE: 
				inside_tex = assets::texture_id::SPELL_HASTE_ICON; 
				border_col = green_spell_color;
				break;

			case spell_type::FURY_OF_THE_AEONS: 
				inside_tex = assets::texture_id::SPELL_FURY_OF_THE_AEONS_ICON; 
				border_col = blue_spell_border;
				break;

			case spell_type::ELECTRIC_TRIAD: 
				inside_tex = assets::texture_id::SPELL_ELECTRIC_TRIAD_ICON;
				border_col = blue_spell_border;
				break;

			case spell_type::ULTIMATE_WRATH_OF_THE_AEONS: 
				inside_tex = assets::texture_id::SPELL_ULTIMATE_WRATH_OF_THE_AEONS_ICON; 
				border_col = blue_spell_border;
				break;

			default: break;
			}

			if (inside_tex != assets::texture_id::INVALID) {
				ensure(border_tex != assets::texture_id::INVALID);

				const augs::gui::material inside_mat(inside_tex, inside_col);
				const augs::gui::material border_mat(border_tex, border_col);

				draw_centered_texture(context, this_id, info, inside_mat);
				
				//augs::gui::solid_stroke stroke;
				//stroke.set_material(border_mat);
				//stroke.set_width(1);
				//stroke.draw(info.v, context.get_tree_entry(this_id).get_absolute_rect());

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

				bound_key_caption.bottom_right(context.get_tree_entry(this_id).get_absolute_rect());
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

			bound_key_caption.center(context.get_tree_entry(this_id).get_absolute_rect());
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
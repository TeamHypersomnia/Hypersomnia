#pragma once
#include "augs/pad_bytes.h"

#include "augs/audio/sound_source.h"

#include "augs/gui/appearance_detector.h"
#include "augs/gui/button_corners.h"

#include "game/assets/ids/asset_ids.h"

#include "augs/drawing/drawing.hpp"
#include "application/gui/menu/appearing_text.h"

using button_corners_info = basic_button_corners_info<assets::necessary_image_id>;

template <class Enum>
class option_button : public menu_rect_node<Enum> {
public:
	using base_node = menu_rect_node<Enum>;
	using gui_entropy = typename base_node::gui_entropy;

	augs::gui::appearance_detector detector;

	augs::sound_source hover_sound;
	augs::sound_source click_sound;

	appearing_text appearing_caption;
	button_corners_info corners = { assets::necessary_image_id::MENU_BUTTON };
	float elapsed_hover_time_ms = 0.f;

	float hover_highlight_maximum_distance = 8.f;
	float hover_highlight_duration_ms = 400.f;

	rgba colorize = white;
	bool click_callback_required = false;
	bool is_discord = false;
	pad_bytes<2> pad;

	template <class M>
	vec2i get_target_button_size(const M& manager, const augs::baked_font& gui_font) const {
		if (is_discord) {
			return manager.at(assets::necessary_image_id::DISCORD_BUTTON).get_original_size();
		}

		return corners.internal_size_to_cornered_size(
			manager,
			augs::gui::text::get_text_bbox({ appearing_caption.get_total_target_text(), gui_font } )
		) - vec2i(0, 3);
	}

	void set_complete_caption(const std::string& text) {
		set_appearing_caption(text);
		make_complete();
	}

	void set_appearing_caption(const std::string& text) {
		appearing_caption.population_interval = 100.f;

		appearing_caption.should_disappear = false;
		appearing_caption.target_text[0] = text;
	}

	void make_complete() {
		appearing_caption.text = appearing_caption.target_text[0];
		appearing_caption.alpha = 255;
	}

	template <class C, class D>
	static void advance_elements(const C context, const D this_id, const augs::delta dt) {
		this_id->click_sound.set_gain(context.get_audio_volume().get_sound_effects_volume());
		this_id->hover_sound.set_gain(context.get_audio_volume().get_sound_effects_volume());

		if (this_id->detector.is_hovered) {
			this_id->elapsed_hover_time_ms += dt.in_milliseconds();
		}

		if (this_id->detector.current_appearance == augs::gui::appearance_detector::appearance::pushed) {
			this_id->elapsed_hover_time_ms = this_id->hover_highlight_duration_ms;
		}
	}

	template <class C, class D>
	static void respond_to_events(const C context, const D this_id, const gui_entropy& entropies) {
		const auto& sounds = context.get_sounds();

		for (const auto& info : entropies.get_events_for(this_id)) {
			this_id->detector.update_appearance(info);

			if (info.is_ldown_or_double_or_triple()) {
				this_id->click_callback_required = true;
				this_id->click_sound.just_play(sounds.button_click);
			}
			if (info.msg == gui_event::hover) {
				this_id->elapsed_hover_time_ms = 0.f;
				this_id->hover_sound.just_play(sounds.button_hover);
			}
		}
	}

	template <class C, class D>
	static void draw(const C context, const D this_id) {
		if (!this_id->get_flag(augs::gui::flag::ENABLE_DRAWING)) {
			return;
		}

		const auto& this_tree_entry = context.get_tree_entry(this_id);
		const auto& necessarys = context.get_necessary_images();
		const auto& gui_font = context.get_gui_font();
		const auto output = context.get_output();
		const auto color = this_id->colorize;

		const auto& detector = this_id->detector;

		rgba inside_col = white;
		rgba border_col = white;

		inside_col.a = 20;
		border_col.a = 190;

		if (detector.is_hovered) {
			inside_col.a = 30;
			border_col.a = 220;
		}

		const bool pushed = detector.current_appearance == augs::gui::appearance_detector::appearance::pushed;

		if (pushed) {
			inside_col.a = 60;
			border_col.a = 255;
		}

		inside_col *= color;
		border_col *= color;

		const auto flip = this_id->corners.flip;
		const auto internal_rc = this_id->corners.cornered_rc_to_internal_rc(necessarys, this_tree_entry.get_absolute_rect());

		{
			this_id->corners.for_each_button_corner(
				necessarys,
				internal_rc, 
				[&](const button_corner_type type, const assets::necessary_image_id id, const ltrb drawn_rc) {
					if (is_lb_complement(type)) { return; }
					const auto col = is_button_border(type) ? border_col : inside_col;
					output.aabb(necessarys.at(id), drawn_rc, col, flip);
				}
			);

			if (this_id->detector.is_hovered) {
				auto hover_effect_rc = internal_rc;

				if (pushed) {
					const auto distance = 4.f;
					hover_effect_rc.expand_from_center(vec2(distance, distance));

					this_id->corners.for_each_button_corner(
						necessarys,
						hover_effect_rc,
						[&](const button_corner_type type, const assets::necessary_image_id id, const ltrb drawn_rc) {
							if (is_lb_complement(type)) { return; }
							if (is_button_border(type)) {
								output.aabb(necessarys.at(id), drawn_rc, color, flip);
							}
						}
					);
				}
				else {
					const auto max_duration = this_id->hover_highlight_duration_ms;
					const auto max_distance = this_id->hover_highlight_maximum_distance;

					const auto distance = (1.f - std::min(max_duration, this_id->elapsed_hover_time_ms) / max_duration) * max_distance;
					hover_effect_rc.expand_from_center(vec2(distance, distance));

					this_id->corners.for_each_button_corner(
						necessarys,
						hover_effect_rc, 
						[&](const button_corner_type type, const assets::necessary_image_id id, const ltrb drawn_rc) {
							if (is_lb_complement(type)) { return; }
							if (is_button_corner(type) && is_button_border(type)) {
								output.aabb(necessarys.at(id), drawn_rc, color, flip);
							}
						}
					);
				}
			}
		}

		this_id->appearing_caption.draw(
			output, 
			internal_rc.left_top(),
			{ gui_font , color }
		);

		if (this_id->is_discord) {
			output.aabb(necessarys.at(assets::necessary_image_id::DISCORD_BUTTON), this_tree_entry.get_absolute_rect(), white);
		}
	}
};
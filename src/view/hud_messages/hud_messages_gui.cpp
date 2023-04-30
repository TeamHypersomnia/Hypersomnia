#include "view/hud_messages/hud_messages_gui.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/logic_step.h"
#include "game/messages/hud_message.h"
#include "augs/misc/time_utils.h"
#include "augs/graphics/renderer.h"
#include "augs/gui/text/printer.h"
#include "view/hud_messages/hud_message_settings.h"
#include "view/faction_view_settings.h"
#include "augs/templates/container_templates.h"
#include "augs/log.h"

void hud_messages_gui::standard_post_solve(
	const const_logic_step step,
	const faction_view_settings& settings,
	const hud_message_settings& hud_settings
) {
	using namespace augs::gui::text;

	const auto& new_messages = step.get_queue<messages::hud_message>();

	for (const auto& new_message : new_messages) {
		const auto& p = new_message.payload;

		std::visit([&](const auto& typed_payload) {
			using T = remove_cref<decltype(typed_payload)>;

			auto msg = hud_message();

			if constexpr(std::is_same_v<T, formatted_string>) {
				msg.text = typed_payload;
			}
			else if constexpr(std::is_same_v<T, messages::special_hud_command>) {
				switch (typed_payload) {
					case messages::special_hud_command::CLEAR:
						messages.clear();
						break;
				}

				return;
			}
			else {
				auto total_text = formatted_string();
				auto& placeholder = augs::baked_font::zero;

				auto add_text = [&](const auto& str, const auto col, bool bbcode = false) {
					if (bbcode) {
						total_text += from_bbcode(str, style(placeholder, col));
					}
					else {
						total_text += formatted_string(str, style(placeholder, col));
					}
				};

				add_text(typed_payload.preffix, hud_settings.text_color, typed_payload.bbcode);
				add_text(typed_payload.first_name, settings.colors[typed_payload.first_faction].current_player_text);
				add_text(typed_payload.mid, hud_settings.text_color, typed_payload.bbcode);
				add_text(typed_payload.second_name, settings.colors[typed_payload.second_faction].current_player_text);
				add_text(typed_payload.suffix, hud_settings.text_color, typed_payload.bbcode);

				msg.text = total_text;
			}

			messages.push_back(msg);
		}, p);
	}
}

void hud_messages_gui::advance(const hud_message_settings& settings) {
	const auto now = augs::date_time().secs_since_epoch();
	const auto total_lifetime = settings.message_lifetime_secs + settings.message_fading_secs;

	if (messages.size() > 0) {
		const auto& m = messages.front();

		if (m.first_appeared.has_value()) {
			const auto passed = now - *m.first_appeared;

			if (passed >= total_lifetime) {
				messages.erase(messages.begin());
			}
		}
	}
}

void hud_messages_gui::draw(
	augs::renderer& renderer,
	const augs::atlas_entry blank_tex,
	const augs::baked_font& font,
	const vec2i screen_size,
	const hud_message_settings& settings,
	const augs::delta& frame_delta
) const {
	using namespace augs::gui::text;

	auto drawer = augs::drawer_with_default { renderer.get_triangle_buffer(), blank_tex };
	const auto line_h = static_cast<int>(font.metrics.get_height());
	const auto now = augs::date_time().secs_since_epoch();

	const auto box_padding = settings.box_padding;
	const auto separation = float(line_h + box_padding.y * 2) * settings.box_separation;
	const auto fading_height = separation;

	// TODO: use this?
	auto fading_mult_of_previous = 0.f;
	(void)fading_mult_of_previous;

	bool is_any_fading = false;

	for (int i = 0; i < settings.max_simultaneous_messages; ++i) {
		if (!(i < static_cast<int>(messages.size()))) {
			break;
		}

		const auto target_spatial_index = float(i);
		const auto& msg = messages[i];

		if (msg.first_appeared == std::nullopt && !is_any_fading) {
			msg.first_appeared = now;
			msg.spatial_index = target_spatial_index;
		}

		if (msg.first_appeared == std::nullopt)
		{
			continue;
		}

		const auto passed = now - *msg.first_appeared;

		const auto appearing_mult = std::min(1.f, (std::sqrt(float(passed / settings.message_fading_secs))));
		const auto fading_progress = passed - settings.message_lifetime_secs;

		auto target_text_alpha = appearing_mult;

		//const auto last_fading_mult = fading_mult_of_previous;

		if (fading_progress >= 0.0f) {
			is_any_fading = true;
			const auto fading_mult = std::sqrt(fading_progress / settings.message_fading_secs);
			fading_mult_of_previous = fading_progress / settings.message_fading_secs;

			const auto alpha_mult = 1.0f - fading_mult;
			target_text_alpha = alpha_mult;
		}

		auto text = msg.text;

		for (auto& t : text) {
			t.format.font = std::addressof(font);
			t.format.color.mult_alpha(target_text_alpha);
		}

		const auto appearing_offset = (1.0f - appearing_mult) * fading_height;

		const auto fading_offset = [&]() {
			if (fading_progress >= 0.f) {
				return (1.0f - target_text_alpha) * fading_height;
			}

			return 0.0f;
		}();

		if (fading_progress < 0.f && msg.spatial_index > target_spatial_index) {
			if (msg.spatial_index - target_spatial_index  < 0.01f) {
				msg.spatial_index = target_spatial_index;
			}
			else {
				msg.spatial_index = augs::interp(msg.spatial_index, target_spatial_index, 1.0f - std::pow(0.5, 20.f * frame_delta.in_seconds()));
			}
		}

		const auto slot_y = -(msg.spatial_index * separation);//+ (last_fading_mult * separation);
		const auto text_pos = vec2i(screen_size.x / 2, slot_y - appearing_offset + fading_offset + float(screen_size.y) * settings.offset_mult);

		const auto black_alpha = target_text_alpha*target_text_alpha*target_text_alpha;

		auto border_col = black;
		border_col.mult_alpha(black_alpha);

		const auto bbox = vec2(get_text_bbox(text)) + box_padding;

		const auto box_alpha = target_text_alpha*target_text_alpha;

		drawer.aabb_with_border(ltrb(text_pos - bbox / 2, bbox), rgba(settings.background_color).mult_alpha(box_alpha), rgba(settings.background_border_color).mult_alpha(box_alpha));

		print_stroked(
			drawer,
			text_pos,
			text,
			{ augs::ralign::CX, augs::ralign::CY },
			border_col
		);
	}
}

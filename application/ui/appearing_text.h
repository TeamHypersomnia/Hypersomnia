#pragma once
#include "augs/gui/text_drawer.h"
#include "augs/gui/formatted_text.h"
#include "augs/gui/text/caret.h"
#include "augs/misc/action_list.h"
#include "augs/misc/standard_actions.h"
namespace augs {
	class action;
}

typedef std::unique_ptr<augs::action> act;

struct appearing_text {
	typedef augs::gui::text::formatted_string formatted_string;

	augs::gui::text_drawer drawer;
	augs::gui::text::style st = augs::gui::text::style(assets::font_id::GUI_FONT, cyan);
	rgba_channel alpha = 0;

	formatted_string text;

	std::array<formatted_string, 2> target_text;

	formatted_string get_total_target_text() const {
		return target_text[0] + target_text[1];
	}

	bool caret_active = false;
	bool should_disappear = true;
	float population_variation = 0.4f;
	float population_interval = 150.f;

	vec2 target_pos;

	void push_disappearance(augs::action_list& into, const bool blocking = true) {
		auto push = [&](act a) {
			blocking ? into.push_blocking(std::move(a)) : into.push_non_blocking(std::move(a));
		};

		push(act(new augs::delay_action(1000.f)));
		push(act(new augs::tween_value_action<rgba_channel>(alpha, 0, 2000.f)));
		push(act(new augs::delay_action(500.f)));
	}

	void push_actions(augs::action_list& into, size_t& rng) {
		auto push = [&](act a) {
			into.push_blocking(std::move(a));
		};

		push(act(new augs::set_value_action<rgba_channel>(alpha, 255)));
		push(act(new augs::set_value_action<formatted_string>(text, formatted_string())));
		push(act(new augs::set_value_action<bool>(caret_active, true)));

		push(act(new augs::populate_with_delays<formatted_string>(text, target_text[0], population_interval * target_text[0].length(), population_variation, rng++)));

		if (target_text[1].size() > 0) {
			push(act(new augs::delay_action(1000.f)));
			push(act(new augs::populate_with_delays<formatted_string>(text, target_text[1], population_interval * target_text[1].length(), population_variation, rng++)));
		}

		push(act(new augs::delay_action(1000.f)));
		push(act(new augs::set_value_action<bool>(caret_active, false)));

		if (should_disappear) {
			push_disappearance(into);
		}
	}

	bool should_draw() const {
		return caret_active || (text.size() > 0 && alpha > 0);
	}

	void draw(augs::vertex_triangle_buffer& buf) {
		if (!should_draw()) {
			return;
		}

		text = set_alpha(text, alpha);
		drawer.pos = target_pos;
		drawer.set_text(text);

		augs::gui::text::caret_info in(st);
		in.pos = text.size();

		auto stroke_color = black;
		stroke_color.a = alpha;
		drawer.print.active = caret_active;
		drawer.draw_stroke(buf, stroke_color);
		drawer.draw(buf, &in);
	}
};
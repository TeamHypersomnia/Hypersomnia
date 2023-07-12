#include <algorithm>

#include "augs/log.h"
#include "augs/string/string_templates.h"
#include "augs/templates/corresponding_field.h"
#include "augs/templates/algorithm_templates.h"

#include "augs/window_framework/window.h"
#include "augs/window_framework/platform_utils.h"

#include "augs/window_framework/mouse_rel_bound.h"

/* Common interface */

namespace augs {
	event::change window::do_raw_motion(basic_vec2<short> motion) {
		/* Clamp it out of consideration for network */
		motion.x = std::clamp(motion.x, mouse_rel_min_v, mouse_rel_max_v);
		motion.y = std::clamp(motion.y, mouse_rel_min_v, mouse_rel_max_v);

		event::change change;
		
		change.data.mouse.rel = motion;

		if (!mouse_pos_paused) {
			last_mouse_pos += vec2i(change.data.mouse.rel);
		}

		const auto screen_size = get_screen_size() - vec2i(1, 1);
		last_mouse_pos.clamp_from_zero_to(screen_size);
		change.data.mouse.pos = basic_vec2<short>(last_mouse_pos);
		change.msg = event::message::mousemotion;

		return change;
	}

	std::optional<event::change> window::sync_mouse_on_click_activate(const event::change& new_change) {
		if (/* should_sync_mouse */ new_change.msg == event::message::click_activate
			&& get_current_settings().draws_own_cursor()
		) {
			if (const auto screen_space = find_cursor_pos()) {
				const auto window_pos = get_window_rect().get_position();
				const auto rel_v = basic_vec2<short>((*screen_space - window_pos) - last_mouse_pos);
				return do_raw_motion(rel_v);
			}
		}

		return std::nullopt;
	}

	void window::common_event_handler(event::change ch, local_entropy& output) {
		using namespace event;

		if (ch.msg == message::activate
			|| ch.msg == message::click_activate
		) {
			active = true;
		}
		
		if (ch.msg == message::deactivate) {
			active = false;
		}

		if (ch.msg == message::move || ch.msg == message::resize) {
			current_rect = get_window_rect_impl();
		}

		if (const auto mouse_change = sync_mouse_on_click_activate(ch)) {
			output.push_back(ch);
		}

		if (ch.msg == message::wheel) {
			if (ch.data.scroll.amount != 0) {
				event::change new_ch;
				new_ch.timestamp = ch.timestamp;

				const auto is_up = ch.data.scroll.amount > 0;
				auto times = std::abs(ch.data.scroll.amount);

				while (times--) {
					const auto key = is_up ? keys::key::WHEELUP : keys::key::WHEELDOWN;

					new_ch.msg = message::keydown;
					new_ch.data.key.key = key;

					output.push_back(new_ch);
					++new_ch.timestamp;

					new_ch.msg = message::keyup;
					output.push_back(new_ch);
					++new_ch.timestamp;
				}
			}
		}
	}

	local_entropy window::collect_entropy() {
		local_entropy output;
		collect_entropy(output);
		return output;
	}

	xywhi window::get_window_rect() const {
		return current_rect;
	}

	vec2i window::get_screen_size() const {
		return current_rect.get_size();	
	}

	void window::set_fullscreen_geometry(const bool flag) {
#if USE_GLFW
		return;
#endif
		if (flag) {
			const auto detected_display = get_display();

			LOG_NVPS(detected_display);
 			set_window_rect(detected_display);

 			if (!set_display(detected_display.get_size(), current_settings.bpp)) {
 				LOG("set_display failed!");
 			}
		}
		else {
 			set_window_rect(get_current_settings().make_window_rect());
		}	
	}

	void window::sync_back_into(window_settings& into) {
		if (!current_settings.fullscreen && can_control_window_geometry) {
			const auto rect = get_window_rect();

			into.size = rect.get_size();
			into.position = rect.get_position();
		}
	}

	std::optional<event::change> window::handle_mousemove(const basic_vec2<short> new_pos) {
		event::change change;

		if (!current_settings.draws_own_cursor() && !mouse_pos_paused) {
			change.data.mouse.rel = new_pos - basic_vec2<short>(last_mouse_pos);
			
			last_mouse_pos = new_pos;

			change.data.mouse.pos = basic_vec2<short>(last_mouse_pos);
			change.msg = event::message::mousemotion;
			
			return change;
		}
		else {
			return std::nullopt;
		}
	}

	void window::apply(const window_settings& settings, const bool force) {
		auto changed = [&](auto& field) {
			return !(field == get_corresponding_field(field, settings, current_settings));
		};

		if (force || changed(settings.name)) {
			set_window_name(settings.name);
		}

		if (force || changed(settings.vsync_mode)) {
			set(settings.vsync_mode);
		}

		if (force || changed(settings.border)) {
			set_window_border_enabled(!settings.fullscreen && settings.border);
		}

		if (force || changed(settings.fullscreen)) {
			if (settings.fullscreen) {
				set_window_border_enabled(false);
				set_fullscreen_geometry(true);
				set_fullscreen_hint(true);
			}
			else {
				set_window_border_enabled(settings.border);
				set_fullscreen_geometry(false);
				set_fullscreen_hint(false);
			}
		}

#if !USE_GLFW
		if (!settings.fullscreen) {
			if (force
				|| changed(settings.position)
				|| changed(settings.size)
				|| changed(settings.border)
			) {
				xywhi screen_rect;

				screen_rect.set_position(settings.position);
				screen_rect.set_size(settings.size);
	
				set_window_rect(screen_rect);
			}
		}
#endif
		
		current_settings = settings;

		if (force) {
			current_rect = get_window_rect_impl();
		}
	}

	window_settings window::get_current_settings() const {
		return current_settings;
	}

	void window::set_mouse_pos_paused(const bool flag) {
		if (mouse_pos_paused && !flag) {
			/* Unpause */
			set_cursor_pos(last_mouse_pos);
		}

		mouse_pos_paused = flag;
	}
	
	void window::set_cursor_clipping(const bool flag) {
		if (flag == cursor_clipping) {
			return;
		}

		if (set_cursor_clipping_impl(flag)) {
			cursor_clipping = flag;
		}
	}

	void window::set_cursor_visible(const bool flag) {
		if (flag == cursor_visible) {
			return;
		}

		set_cursor_visible_impl(flag);

		cursor_visible = flag;
	}

	bool window::is_mouse_pos_paused() const {
		return mouse_pos_paused;
	}

	bool window::is_active() const {
		return active;
	}
}

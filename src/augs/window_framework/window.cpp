#include <algorithm>

#include "augs/log.h"

#include "augs/templates/string_templates.h"
#include "augs/templates/corresponding_field.h"
#include "augs/templates/algorithm_templates.h"

#include "augs/window_framework/window.h"
#include "augs/window_framework/platform_utils.h"

/* Common interface */

namespace augs {
	vec2i window_settings::get_screen_size() const {
		return fullscreen ? augs::get_display().get_size() : size;
	}
	
	local_entropy window::collect_entropy() {
		local_entropy output;
		collect_entropy(output);
		return output;
	}

	vec2i window::get_screen_size() const {
		return get_window_rect().get_size();	
	}

	void window::sync_back_into(window_settings& into) {
		if (!current_settings.fullscreen) {
			into.size = get_window_rect().get_size();
			into.position = get_window_rect().get_position();
		}
	}

	void window::apply(const window_settings& settings, const bool force) {
		auto changed = [&](auto& field) {
			return !(field == get_corresponding_field(field, settings, current_settings));
		};

		if (force || changed(settings.name)) {
			set_window_name(settings.name);
		}

		if (force || changed(settings.position)) {
			auto r = get_window_rect();
			r.set_position(settings.position);
			set_window_rect(r);
		}

		if (force || changed(settings.border)) {
			set_window_border_enabled(!settings.fullscreen && settings.border);
		}

		if (force || changed(settings.fullscreen)) {
			if (settings.fullscreen) {
				set_display(settings.get_screen_size(), settings.bpp);
				set_window_border_enabled(false);
			}
			else {
				set_window_border_enabled(settings.border);
			}
		}

		if (force
			|| settings.get_screen_size() != current_settings.get_screen_size()
			|| changed(settings.border)
		) {
			xywhi screen_rect;

			if (!settings.fullscreen) {
				screen_rect.set_position(settings.position);
			}

			screen_rect.set_size(settings.get_screen_size());
			set_window_rect(screen_rect);
		}

		current_settings = settings;
	}

	window_settings window::get_current_settings() const {
		return current_settings;
	}

	window::~window() {
		destroy();
	}
}

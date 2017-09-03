#include <thread>

#include "game/transcendental/cosmos.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/cosmic_movie_director.h"

#include "augs/templates/string_templates.h"
#include "augs/filesystem/file.h"
#include "augs/window_framework/log_color.h"
#include "local_setup.h"
#include "game/detail/visible_entities.h"

#include "generated/introspectors.h"

using namespace augs::event::keys;

void local_setup::control(
	augs::local_entropy& new_entropy,
	const input_context& context
) {
	const bool debug_control_timing = true;// player.is_replaying();

	if (debug_control_timing) {
		for (const auto& raw_input : new_entropy) {
			if (raw_input.was_any_key_pressed()) {
				if (raw_input.key == key::_4) {
					timer.set_stepping_speed_multiplier(0.1f);
				}
				if (raw_input.key == key::_5) {
					timer.set_stepping_speed_multiplier(1.f);
				}
				if (raw_input.key == key::_6) {
					timer.set_stepping_speed_multiplier(6.f);
				}
			}
		}
	}

	for (const auto& raw_input : new_entropy) {
		if (raw_input.was_any_key_pressed()) {
			if (raw_input.key == key::F2) {
				LOG_COLOR(console_color::YELLOW, "Separator");
			}
		}
	}

	characters.control_character_selection_numeric(new_entropy);

	auto translated = context.translate(new_entropy);
	characters.control_character_selection(translated.intents);

	control(cosmic_entropy(
		get_viewed_character(),
		translated
	));
}

void local_setup::control(
	const cosmic_entropy& entropy
) {
	total_collected_entropy += entropy;
}
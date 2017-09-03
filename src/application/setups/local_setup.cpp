#include "augs/templates/string_templates.h"
#include "augs/filesystem/file.h"
#include "augs/window_framework/log_color.h"

#include "game/detail/visible_entities.h"

#include "game/hardcoded_content/test_scenes/test_scenes_content.h"

#include "game/hardcoded_content/test_scenes/scenes/testbed.h"
#include "game/hardcoded_content/test_scenes/scenes/minimal_scene.h"

#include "game/transcendental/logic_step.h"
#include "game/transcendental/types_specification/all_messages_includes.h"

#include "application/setups/local_setup.h"

#include "generated/introspectors.h"

using namespace augs::event::keys;

local_setup::local_setup(
	const bool make_minimal_test_scene,
	const input_recording_type recording_type
) {
#if BUILD_TEST_SCENES
	hypersomnia.reserve_storage_for_entities(3000u);

	populate_test_scene_assets(logical_assets, viewable_defs);

	if (make_minimal_test_scene) {
		test_scenes::minimal_scene().populate_world_with_entities(
			hypersomnia,
			logical_assets,
			[](auto...){}
		);
	}
	else {
		test_scenes::testbed().populate_world_with_entities(
			hypersomnia,
			logical_assets,
			[](auto...) {}
		);
	}

	characters.acquire_available_characters(hypersomnia);
#endif

	if (recording_type != input_recording_type::DISABLED) {
		if (player.try_to_load_or_save_new_session("generated/sessions/", "recorded.inputs")) {
			timer.set_stepping_speed_multiplier(1.f);
		}
	}

	timer.reset_timer();
}

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

	total_collected_entropy += cosmic_entropy(
		get_viewed_character(),
		translated
	);
}

void local_setup::accept_game_gui_events(const cosmic_entropy& events) {
	total_collected_entropy += events;
}
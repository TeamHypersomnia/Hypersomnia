#pragma once
#include <cstdint>
#include "augs/templates/container_templates.h"
#include "application/setups/editor/playtest_recording/editor_playtest_recording.h"

template <class Commands>
void keep_only_key_releases(Commands& commands) {
	auto releases_only = Commands();
	releases_only.intents = commands.intents;

	erase_if(releases_only.intents, [](const auto& intent) {
		return !intent.was_released();
	});

	commands = releases_only;
}

inline void cut_off_recorded_inputs_from(editor_playtest_recording_inputs& inputs, const uint32_t first_step_without_input) {
	for (auto& [step, entropy] : inputs.entropies) {
		if (step < first_step_without_input) {
			continue;
		}

		for (auto& [character, player] : entropy.cosmic.players) {
			::keep_only_key_releases(player.commands);
		}

		entropy.players.clear();
	}
}

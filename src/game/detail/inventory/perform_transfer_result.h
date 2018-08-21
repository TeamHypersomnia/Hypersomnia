#pragma once
#include <optional>

#include "game/messages/queue_deletion.h"
#include "game/messages/item_picked_up_message.h"
#include "game/messages/interpolation_correction_request.h"

#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/view_input/particle_effect_input.h"

struct perform_transfer_result {
	std::optional<messages::queue_deletion> destructed;
	std::vector<messages::interpolation_correction_request> interpolation_corrected;
	std::optional<messages::item_picked_up_message> picked;

	std::optional<packaged_sound_effect> transfer_sound;
	std::optional<packaged_particle_effect> transfer_particles;

	void notify_logical(logic_step) const;
	void notify(logic_step) const;
	void play_effects(logic_step) const;
};


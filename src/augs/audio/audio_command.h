#pragma once
#include <variant>
#include <vector>

#include "augs/audio/audio_commands.h"

namespace augs {
	using audio_command_payload = std::variant<
		update_listener_properties,
		update_multiple_properties,

		update_flash_noise,
		reseek_to_sync_if_needed,

		bind_sound_buffer,
		source_no_arg_command,
		source1f_command
	>;

	struct audio_command {
		audio_command_payload payload;
	};

	using audio_command_buffer = std::vector<augs::audio_command>;
}

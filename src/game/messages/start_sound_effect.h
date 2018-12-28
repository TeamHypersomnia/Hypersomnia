#pragma once
#include <optional>
#include "game/messages/message.h"
#include "augs/math/vec2.h"

#include "game/cosmos/entity_id.h"
#include "game/components/transform_component.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/assets/ids/asset_ids.h"

namespace messages {
	struct start_sound_effect : predicted_message {
		using predicted_message::predicted_message;
		packaged_sound_effect payload;
	};

	struct start_multi_sound_effect : predicted_message {
		using predicted_message::predicted_message;
		packaged_multi_sound_effect payload;
	};

	struct stop_sound_effect : predicted_message {
		using predicted_message::predicted_message;
		std::optional<entity_id> match_chased_subject;
		std::optional<::assets::sound_id> match_effect_id;
	};
}

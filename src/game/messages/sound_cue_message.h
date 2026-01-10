#pragma once
#include "game/messages/message.h"
#include "game/components/transform_component.h"

namespace messages {
	/*
		Posted when a sound cue occurs that AI bots should be able to hear.
		Used for footsteps and other audible cues.
	*/
	struct sound_cue_message : message {
		vec2 position;
		float max_distance = 0.0f;
		entity_id source_entity;
	};
}

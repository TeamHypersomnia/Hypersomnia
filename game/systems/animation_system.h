#pragma once
#include "misc/timer.h"

#include "game/processing_system_with_cosmos_reference.h"

#include "game/components/animation_component.h"
#include "game/components/render_component.h"

using namespace augs;


class animation_system : public processing_system_templated<components::animation, components::render> {
public:
	using processing_system_templated::processing_system_templated;

	void game_responses_to_animation_messages();

	void handle_animation_messages();
	void progress_animation_states();
};
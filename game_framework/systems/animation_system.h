#pragma once
#include "misc/timer.h"

#include "entity_system/processing_system.h"

#include "../components/animation_component.h"
#include "../components/render_component.h"

using namespace augs;


class animation_system : public processing_system_templated<components::animation, components::render> {
public:
	using processing_system_templated::processing_system_templated;

	timer animation_timer;

	void game_responses_to_animation_messages();

	void handle_animation_messages();
	void progress_animation_states();
};
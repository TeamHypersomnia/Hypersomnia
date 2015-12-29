#pragma once
#include "misc/timer.h"

#include "entity_system/processing_system.h"

#include "../components/animate_component.h"
#include "../components/render_component.h"

using namespace augs;


class animation_system : public processing_system_templated<components::animate, components::render> {
public:
	using processing_system_templated::processing_system_templated;

	timer animation_timer;
	void consume_events();
	void process_entities();
};
#pragma once
#include "entity_system/processing_system.h"

#include "../components/chase_component.h"
#include "../components/transform_component.h"

using namespace augmentations;
using namespace entity_system;

class chase_system : public processing_system_templated<components::transform, components::chase> {
public:
	void process_entities(world&) override;
	void add(entity*) override;
};
#pragma once
#include "entity_system/processing_system.h"

#include "../components/crosshair_component.h"
#include "../components/transform_component.h"

using namespace augmentations;
using namespace entity_system;

class crosshair_system : public processing_system_templated<components::transform, components::crosshair> {
public:
	void process_entities(world&) override;
};
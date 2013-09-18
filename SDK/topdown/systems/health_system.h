#pragma once
#include "entity_system/processing_system.h"

#include "../components/health_component.h"

using namespace augmentations;
using namespace entity_system;

class health_system : public processing_system_templated<components::health> {
public:
	void process_entities(world&) override;
};
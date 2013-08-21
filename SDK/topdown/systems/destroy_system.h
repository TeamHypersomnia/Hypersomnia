#pragma once
#include "entity_system/processing_system.h"

using namespace augmentations;
using namespace entity_system;

class destroy_system : public event_only_system_templated<> {
public:
	void process_entities(world&);
};
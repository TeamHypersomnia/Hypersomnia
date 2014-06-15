#pragma once
#include "entity_system/processing_system.h"
#include "../components/scriptable_component.h"

using namespace augs;
using namespace entity_system;

class script_system : public processing_system_templated<components::scriptable> {
public:
	std::vector<luabind::object*> script_entities;
	
	std::vector<luabind::object> get_entities_vector() const;

	void add(entity*) override;
	void remove(entity*) override;
	void clear() override;
};
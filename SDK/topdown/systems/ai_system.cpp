#include "stdafx.h"
#include "ai_system.h"

#include "entity_system/world.h"
#include "entity_system/entity.h"

#include "physics_system.h"

void ai_system::process_entities(world& owner) {
	physics_system& physics = owner.get_system<physics_system>();

	for (auto it : targets) {
		auto& ai = it->get<components::ai>();
	}
}
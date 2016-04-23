#pragma once
#include "entity_system/processing_system.h"
#include "game/components/melee_component.h"
#include "game/components/damage_component.h"

using namespace augs;

enum stage {
	FIRST_STAGE,
	SECOND_STAGE
};

class melee_system : public processing_system_templated<components::melee, components::damage> {
public:
	using processing_system_templated::processing_system_templated;

	void consume_melee_intents();
	void initiate_and_update_moves();
	components::melee_state primary_action(double dt, augs::entity_id& target, components::melee& melee_component);
	components::melee_state secondary_action(double dt, augs::entity_id& target, components::melee& melee_component);
	components::melee_state tertiary_action(double dt, augs::entity_id& target, components::melee& melee_component);
private:
	stage action_stage;
};
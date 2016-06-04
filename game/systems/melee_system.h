#pragma once
#include "entity_system/processing_system.h"
#include "game/components/melee_component.h"
#include "game/components/damage_component.h"

#include "game/detail/combat/melee_animation.h"

using namespace augs;

class melee_system : public processing_system_templated<components::melee, components::damage> {
public:
	using processing_system_templated::processing_system_templated;

	void consume_melee_intents();
	void initiate_and_update_moves();
	components::melee::state primary_action(double dt, augs::entity_id target, components::melee& melee_component, components::damage& damage);
	components::melee::state secondary_action(double dt, augs::entity_id target, components::melee& melee_component, components::damage& damage);
	components::melee::state tertiary_action(double dt, augs::entity_id target, components::melee& melee_component, components::damage& damage);

	melee_animation animation;
private:
	components::melee::stage action_stage = components::melee::stage::FIRST_STAGE;
};
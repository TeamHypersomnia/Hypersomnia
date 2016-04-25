#pragma once
#include "entity_system/processing_system.h"
#include "game/components/melee_component.h"
#include "game/components/damage_component.h"

#include "game/detail/combat/melee_animation.h"

using namespace augs;

enum stage {
	FIRST_STAGE, 
	SECOND_STAGE, 
	THIRD_STAGE, 
	FOURTH_STAGE, 
	WINDOW_STAGE //During the window stage the player can perform the second swing or an other melee action.
};

class melee_system : public processing_system_templated<components::melee, components::damage> {
public:
	using processing_system_templated::processing_system_templated;

	void consume_melee_intents();
	void initiate_and_update_moves();
	components::melee_state primary_action(double dt, augs::entity_id target, components::melee& melee_component, components::damage& damage);
	components::melee_state secondary_action(double dt, augs::entity_id target, components::melee& melee_component, components::damage& damage);
	components::melee_state tertiary_action(double dt, augs::entity_id target, components::melee& melee_component, components::damage& damage);

	melee_animation animation;
private:
	stage action_stage = FIRST_STAGE;
};
#pragma once
#include "game/cosmos/step_declaration.h"
#include "game/cosmos/entity_id.h"
#include "game/detail/transform_copying.h"
#include "src/game/components/transform_component.h"

void fire_and_forget_particle_effect(
	particle_effect_input,
	logic_step,  
	transformr,
	entity_id homing_target = {}
);

void start_orbital_particle_effect(
	particle_effect_input,
	logic_step,  
	entity_id,
	entity_id homing_target = {}
);

void start_orbital_particle_effect(
	particle_effect_input,
	logic_step,  
	transformr world_transform,
	entity_id,
	entity_id homing_target = {}
);

void start_orbital_particle_effect(
	particle_effect_input,
	logic_step,  
	particle_effect_start_input
);

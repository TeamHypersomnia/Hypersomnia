#include "melee_system.h"
#include "game/cosmos.h"
#include "game/entity_id.h"

#include "game/messages/intent_message.h"
#include "game/messages/melee_swing_response.h"
#include "game/detail/inventory_utils.h"

#include "game/components/melee_component.h"
#include "game/components/damage_component.h"
#include "game/components/fixtures_component.h"


#include "game/detail/combat/melee_animation.h"

#include "game/entity_handle.h"
#include "game/step.h"

using namespace augs;

void components::melee::reset_weapon(entity_handle e) {
	auto& m = e.get<components::melee>();
	m.reset_move_flags();
	m.current_state = melee_state::FREE;

	auto& d = e.get<components::damage>();
	d.damage_upon_collision = false;
}

void melee_system::consume_melee_intents(fixed_step& step) {
	auto& cosmos = step.cosm;
	auto& delta = step.get_delta();
	auto& events = step.messages.get_queue<messages::intent_message>();


	for (auto it : events) {
		/* 
			we search the whole intent message queue for events of interest;
			here we can even find gunshot requests for some distant enemy AI,
			therefore we need to filter out events we're interested in, and that would be
			melee-related intents and only these applied to an entity with a melee component
		*/
		auto* maybe_melee = cosmos[it.subject].find<components::melee>();
		
		if (maybe_melee == nullptr) 
			continue;

		auto& melee = *maybe_melee;

		if (it.intent == intent_type::MELEE_PRIMARY_MOVE) {
			melee.primary_move_flag = it.pressed_flag;
		}

		if (it.intent == intent_type::MELEE_SECONDARY_MOVE) {
			melee.secondary_move_flag = it.pressed_flag;
		}
		
		if (it.intent == intent_type::MELEE_TERTIARY_MOVE) {
			melee.tertiary_move_flag = it.pressed_flag;
		}
	}
}

void melee_system::initiate_and_update_moves(fixed_step& step) {
	auto& cosmos = step.cosm;
	auto& delta = step.get_delta();
	/* 
	- fixed delta timestep if it's a logic procedure 
	- variable frame time if it is a rendering-time procedure 
	*/
	auto dt = cosmos.delta.in_milliseconds();

	/* clear melee swing response queue because this is the only place where we send them */
	step.messages.get_queue<messages::melee_swing_response>().clear();

	/* all entities in the "targets" vector are guaranteed to have both melee and damage components
		therefore we use get instead of find
	*/

	auto targets_copy = cosmos.get(processing_subjects::WITH_MELEE);

	for (auto& t : targets_copy) {
		auto& melee = t.get<components::melee>();
		auto& damage = t.get<components::damage>();

		//		 LOG("P: %x, S: %x, T: %x CDT: %x MVT: %x STATE: %x", melee.primary_move_flag, melee.secondary_move_flag, melee.tertiary_move_flag,melee.swing_current_cooldown_time,melee.swing_current_time,melee.state);

		switch (melee.current_state) {
		case melee_state::FREE:
			if (melee.primary_move_flag) {
				melee.current_state = primary_action(cosmos, step, dt, t, melee, damage);
			}
			/* send a response message so that the rest of the game knows that a swing has occured;
			the message could be read by particles system, audio system and possibly animation system
			to apply each their own effects.
			*/
			break;
		case melee_state::ONCOOLDOWN:
			melee.swing_current_cooldown_time += dt;
			if (melee.swing_current_cooldown_time >= melee.swings[0].cooldown_ms) {
				melee.swing_current_cooldown_time = 0;
				melee.current_state = melee_state::FREE;
			}
			break;
		case melee_state::PRIMARY:
			melee.current_state = primary_action(cosmos,step,dt,t,melee,damage);
			break;
		 default:
			LOG("Uknown action in melee_system.cpp");
		}
	}
}

melee_state melee_system::primary_action(fixed_step& step, double dt, entity_handle target, components::melee& melee_component, components::damage& damage)
{
	damage.damage_upon_collision = true;

	if (melee_component.swing_current_time > melee_component.swings[int(melee_component.action_stage)].duration_ms) {
		switch (melee_component.action_stage)
		{
		case components::melee::stage::FIRST_STAGE:
			melee_component.action_stage = components::melee::stage::SECOND_STAGE;
			melee_component.swing_current_time = 0;
			break;
		case components::melee::stage::SECOND_STAGE:
			melee_component.action_stage = components::melee::stage::WINDOW_STAGE;
			melee_component.swing_current_time = 0;
			return melee_state::PRIMARY;
			break;
		case components::melee::stage::THIRD_STAGE:
			melee_component.action_stage = components::melee::stage::FOURTH_STAGE;
			melee_component.swing_current_time = 0;
			break;
		case components::melee::stage::FOURTH_STAGE:
			melee_component.action_stage = components::melee::stage::FIRST_STAGE;
			melee_component.swing_current_time = 0;
			return melee_state::ONCOOLDOWN;
			break;
		case components::melee::stage::WINDOW_STAGE:
			damage.damage_upon_collision = false;
			melee_component.window_current_time += dt;
			if (melee_component.window_current_time >= melee_component.window_time) {
				melee_component.action_stage = components::melee::stage::FIRST_STAGE;
				melee_component.window_current_time = 0;
				return melee_state::ONCOOLDOWN;
			}
			else if (melee_component.primary_move_flag && melee_component.window_current_time >= melee_component.swings[0].cooldown_ms) {
				melee_component.action_stage = components::melee::stage::THIRD_STAGE;
				return melee_state::PRIMARY;
			}
			else if (melee_component.secondary_move_flag && melee_component.window_current_time >= melee_component.swings[0].cooldown_ms) {
				melee_component.action_stage = components::melee::stage::FIRST_STAGE;
				return melee_state::SECONDARY;
			}
			else if (melee_component.tertiary_move_flag && melee_component.window_current_time >= melee_component.swings[0].cooldown_ms) {
				melee_component.action_stage = components::melee::stage::FIRST_STAGE;
				return melee_state::TERTIARY;
			}
			else {
				return melee_state::PRIMARY;
			}
			break;
		default:
			melee_component.swing_current_time = 0;
			melee_component.action_stage = components::melee::stage::FIRST_STAGE;
			return melee_state::ONCOOLDOWN;
		}
	}

	melee_component.swing_current_time += dt * melee_component.swings[int(melee_component.action_stage)].acceleration;

	messages::melee_swing_response response;

	auto& m = target.get<components::fixtures>();

	//auto new_definition = target.get<components::physics_definition>();

	melee_animation animation;

	animation.offset_pattern = melee_component.offset_positions[int(melee_component.action_stage)];
	m.set_offset(components::fixtures::offset_type::SPECIAL_MOVE_DISPLACEMENT, animation.calculate_intermediate_transform(melee_component.swing_current_time / melee_component.swings[int(melee_component.action_stage)].duration_ms));
	auto player = cosmos[get_owning_transfer_capability(target)];
	damage.custom_impact_velocity = target.get<components::transform>().pos - player.get<components::transform>().pos;

	response.subject = target;
	response.origin_transform = target.get<components::transform>();
	step.messages.post(response);

	//pos_response.new_definition = new_definition;
	//step.messages.post(pos_response);

	return melee_state::PRIMARY;
}
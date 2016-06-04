#include "melee_system.h"
#include "entity_system/world.h"
#include "entity_system/entity.h"

#include "game/messages/intent_message.h"
#include "game/messages/melee_swing_response.h"
#include "game/messages/rebuild_physics_message.h"
#include "game/detail/inventory_utils.h"

void components::melee::reset_weapon(augs::entity_id e) {
	auto& m = e->get<components::melee>();
	m.reset_move_flags();
	m.current_state = components::melee::state::FREE;

	auto& d = e->get<components::damage>();
	d.damage_upon_collision = false;
}

void melee_system::consume_melee_intents() {
	auto& events = parent_world.get_message_queue<messages::intent_message>();

	for (auto it : events) {
		/* 
			we search the whole intent message queue for events of interest;
			here we can even find gunshot requests for some distant enemy AI,
			therefore we need to filter out events we're interested in, and that would be
			melee-related intents and only these applied to an entity with a melee component
		*/
		auto* maybe_melee = it.subject->find<components::melee>();
		
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

void melee_system::initiate_and_update_moves() {
	/* 
	- fixed delta timestep if it's a logic procedure 
	- variable frame time if it is a rendering-time procedure 
	*/
	auto dt = delta_milliseconds();

	/* clear melee swing response queue because this is the only place where we send them */
	parent_world.get_message_queue<messages::melee_swing_response>().clear();

	/* all entities in the "targets" vector are guaranteed to have both melee and damage components
		therefore we use get instead of find
	*/
	for (auto& t : targets) {
		auto& melee = t->get<components::melee>();
		auto& damage = t->get<components::damage>();

		//		 LOG("P: %x, S: %x, T: %x CDT: %x MVT: %x STATE: %x", melee.primary_move_flag, melee.secondary_move_flag, melee.tertiary_move_flag,melee.swing_current_cooldown_time,melee.swing_current_time,melee.state);

		switch (melee.current_state) {
		case components::melee::state::FREE:
			if (melee.primary_move_flag) {
				melee.current_state = primary_action(dt, t, melee, damage);
			}
			/* send a response message so that the rest of the game knows that a swing has occured;
			the message could be read by particles system, audio system and possibly animation system
			to apply each their own effects.
			*/
			break;
		case components::melee::state::ONCOOLDOWN:
			melee.swing_current_cooldown_time += dt;
			if (melee.swing_current_cooldown_time >= melee.swings[0].cooldown_ms) {
				melee.swing_current_cooldown_time = 0;
				melee.current_state = components::melee::state::FREE;
			}
			break;
		case components::melee::state::PRIMARY:
			melee.current_state = primary_action(dt,t,melee,damage);
			break;
		 default:
			LOG("Uknown action in melee_system.cpp");
		}
	}
}

components::melee::state melee_system::primary_action(double dt, augs::entity_id target, components::melee& melee_component, components::damage& damage)
{
	damage.damage_upon_collision = true;

	if (melee_component.swing_current_time > melee_component.swings[int(action_stage)].duration_ms) {
		switch (action_stage)
		{
		case components::melee::stage::FIRST_STAGE:
			action_stage = components::melee::stage::SECOND_STAGE;
			melee_component.swing_current_time = 0;
			break;
		case components::melee::stage::SECOND_STAGE:
			action_stage = components::melee::stage::WINDOW_STAGE;
			melee_component.swing_current_time = 0;
			return components::melee::state::PRIMARY;
			break;
		case components::melee::stage::THIRD_STAGE:
			action_stage = components::melee::stage::FOURTH_STAGE;
			melee_component.swing_current_time = 0;
			break;
		case components::melee::stage::FOURTH_STAGE:
			action_stage = components::melee::stage::FIRST_STAGE;
			melee_component.swing_current_time = 0;
			return components::melee::state::ONCOOLDOWN;
			break;
		case components::melee::stage::WINDOW_STAGE:
			damage.damage_upon_collision = false;
			melee_component.window_current_time += dt;
			if (melee_component.window_current_time >= melee_component.window_time) {
				action_stage = components::melee::stage::FIRST_STAGE;
				melee_component.window_current_time = 0;
				return components::melee::state::ONCOOLDOWN;
			}
			else if (melee_component.primary_move_flag && melee_component.window_current_time >= melee_component.swings[0].cooldown_ms) {
				action_stage = components::melee::stage::THIRD_STAGE;
				return components::melee::state::PRIMARY;
			}
			else if (melee_component.secondary_move_flag && melee_component.window_current_time >= melee_component.swings[0].cooldown_ms) {
				action_stage = components::melee::stage::FIRST_STAGE;
				return components::melee::state::SECONDARY;
			}
			else if (melee_component.tertiary_move_flag && melee_component.window_current_time >= melee_component.swings[0].cooldown_ms) {
				action_stage = components::melee::stage::FIRST_STAGE;
				return components::melee::state::TERTIARY;
			}
			else {
				return components::melee::state::PRIMARY;
			}
			break;
		default:
			melee_component.swing_current_time = 0;
			action_stage = components::melee::stage::FIRST_STAGE;
			return components::melee::state::ONCOOLDOWN;
		}
	}

	melee_component.swing_current_time += dt * melee_component.swings[int(action_stage)].acceleration;

	messages::rebuild_physics_message pos_response;
	messages::melee_swing_response response;

	pos_response.subject = target;
	auto new_definition = target->get<components::physics_definition>();

	animation.offset_pattern = melee_component.offset_positions[int(action_stage)];
	new_definition.offsets_for_created_shapes[components::physics_definition::SPECIAL_MOVE_DISPLACEMENT] = animation.calculate_intermediate_transform(melee_component.swing_current_time / melee_component.swings[int(action_stage)].duration_ms);
	auto player = get_owning_transfer_capability(target);
	damage.custom_impact_velocity = target->get<components::transform>().pos - player->get<components::transform>().pos;

	response.subject = target;
	response.origin_transform = target->get<components::transform>();
	parent_world.post_message(response);

	pos_response.new_definition = new_definition;
	parent_world.post_message(pos_response);

	return components::melee::state::PRIMARY;
}
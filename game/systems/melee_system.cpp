#include "melee_system.h"
#include "entity_system/world.h"
#include "entity_system/entity.h"

#include "game/messages/intent_message.h"
#include "game/messages/melee_swing_response.h"

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

		LOG("P: %x, S: %x, T: %x", melee.primary_move_flag, melee.secondary_move_flag, melee.tertiary_move_flag);

		damage.damage_upon_collision = melee.primary_move_flag;

		if (melee.primary_move_flag) {
			/* send a response message so that the rest of the game knows that a swing has occured;
			the message could be read by particles system, audio system and possibly animation system
			to apply each their own effects.
			*/

			messages::melee_swing_response response;
			response.subject = t;
			response.origin_transform = t->get<components::transform>();
			parent_world.post_message(response);
		}
	}
}
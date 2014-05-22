#include "stdafx.h"

#include "entity_system/world.h"
#include "entity_system/entity.h"
#include "entity_system/entity_ptr.h"

#include "script_system.h"

#include "../components/scriptable_component.h"

#include "../messages/collision_message.h"
#include "../messages/damage_message.h"
#include "../messages/intent_message.h"
#include "../messages/shot_message.h"

void script_system::process_entities(world& owner) {
	call_loop(owner, false);
}

void script_system::call_loop(world& owner, bool substepping_flag) {
	auto target_copy = targets;
	for (auto it : target_copy) {
		auto& scriptable = it->get<components::scriptable>();
		if (!scriptable.available_scripts) continue;

		auto loop_event = scriptable.available_scripts->get_raw().find(components::scriptable::LOOP);
		
		if (loop_event != scriptable.available_scripts->get_raw().end()) {
			try {
				luabind::call_function<void>((*loop_event).second, it, substepping_flag);
			}
			catch (std::exception compilation_error) {
				std::cout << compilation_error.what() << std::endl;
			}
		}
		//auto loop_event = scriptable
	}
}

void script_system::substep(world& owner) {
	call_loop(owner, true);
	pass_events(owner, true);
}

template<typename message_type>
void pass_events_to_script(world& owner, int msg_enum, bool substepping_flag) {
	auto& events = owner.get_message_queue<message_type>();

	std::for_each(events.begin(), events.end(), [msg_enum, substepping_flag](message_type msg){
		auto* scriptable = msg.subject->find<components::scriptable>();
		if (scriptable == nullptr || !scriptable->available_scripts) return;

		auto it = scriptable->available_scripts->get_raw().find(msg_enum);

		if (it != scriptable->available_scripts->get_raw().end()) {
			luabind::call_function<void>((*it).second, msg, substepping_flag);
		}
	});
}
using namespace messages;


void script_system::pass_events(world& owner, bool substepping_flag) {
	try {
		pass_events_to_script<collision_message>(owner, components::scriptable::COLLISION_MESSAGE, substepping_flag);
		pass_events_to_script<damage_message>(owner, components::scriptable::DAMAGE_MESSAGE, substepping_flag);
		pass_events_to_script<intent_message>(owner, components::scriptable::INTENT_MESSAGE, substepping_flag);
		pass_events_to_script<shot_message>(owner, components::scriptable::SHOT_MESSAGE, substepping_flag);
	}
	catch (std::exception compilation_error) {
		std::cout << compilation_error.what() << std::endl;
	}
}

void script_system::process_events(world& owner) {
	pass_events(owner, false);
}
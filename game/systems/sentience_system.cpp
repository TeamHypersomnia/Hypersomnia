#include "sentience_system.h"

#include "game/messages/damage_message.h"
#include "entity_system/world.h"
#include "entity_system/entity.h"

void sentience_system::apply_damage_and_initiate_deaths() {
	auto& damages = parent_world.get_message_queue<messages::damage_message>();

	for (auto& d : damages) {
		auto* sentience = d.subject->find<components::sentience>();

		if (sentience) {
			if (sentience->enable_health) {
				sentience->health -= d.amount;

				if (sentience->health < 0) {
					sentience->health = 0;
				}
			}

			if (sentience->enable_consciousness) {
				sentience->consciousness -= d.amount;

				if (sentience->consciousness < 0) {
					sentience->health = 0;
				}
				
				sentience->consciousness = std::min(sentience->health, sentience->consciousness);
			}
		}
	}
}
#include "data_living_one_step.h"
#include "game/organization/all_messages_includes.h"

void data_living_one_step::flush_everything() {
	messages.flush_queues();

	calculated_visibility.clear();
}

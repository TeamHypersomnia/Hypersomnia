#include "data_living_one_step.h"
#include "game/organization/all_messages_includes.h"

void data_living_one_step::clear_all() {
	messages.flush_queues();

	calculated_visibility.clear();
	calculated_line_of_sight.clear();
}

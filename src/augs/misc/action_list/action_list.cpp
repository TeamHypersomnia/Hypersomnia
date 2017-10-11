#include "action_list.h"
#include "action.h"
#include "augs/misc/timing/delta.h"

namespace augs {
	void action_list::update(const delta dt) {
		size_t i = 0;
		
		while (i < actions.size()) {
			if (!actions[i]->has_started) {
				actions[i]->on_enter();
				actions[i]->has_started = true;
			}

			actions[i]->on_update(dt);

			if (actions[i]->is_complete()) {
				actions.erase(actions.begin() + i);
				continue;
			}
			else {
				if (actions[i]->is_blocking) {
					break;
				}
				else {
					++i;
				}
			}
		}
	}

	void action_list::push_blocking(std::unique_ptr<action> act) {
		act->is_blocking = true;
		actions.emplace_back(std::move(act));
	}

	void action_list::push_non_blocking(std::unique_ptr<action> act) {
		act->is_blocking = false;
		actions.emplace_back(std::move(act));
	}

	bool action_list::is_complete() const {
		return actions.empty();
	}
}
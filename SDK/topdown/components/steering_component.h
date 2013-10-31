#pragma once
#include "entity_system/component.h"
#include "entity_system/entity_ptr.h"
#include "math/vec2d.h"

class steering_system;

namespace components {
	struct steering : public augmentations::entity_system::component {
		typedef augmentations::entity_system::entity_ptr target;
		
		struct behaviour {
			enum {
				SEEK, FLEE, ARRIVAL, PURSUIT
			} behaviour_type;

			target current_target;

			augmentations::vec2<> max_force_applied;
			float weight;
		
			bool erase_when_target_reached;

			behaviour() : weight(1.f), erase_when_target_reached(true) {}
		private:
			friend class steering_system;
			bool completed;
		};

		std::vector<behaviour> active_behaviours;

		/* binding facility */
		void add_behaviour(const behaviour& b) { active_behaviours.push_back(b); }
		void clear_behaviours() { active_behaviours.clear(); }
	};
}
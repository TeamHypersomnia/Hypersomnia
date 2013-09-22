#pragma once
#include "utility/timer.h"
#include "entity_system/component.h"
#include "entity_system/entity_ptr.h"

#include "../resources/gun_info"

class gun_system;
namespace components {
	struct gun : public augmentations::entity_system::component {
		resources::gun_info* info;
		unsigned current_rounds;

		bool reloading, trigger;

		augmentations::entity_system::entity_ptr target_camera_to_shake;

		gun(gun_info* info = nullptr)
			: info(info), current_rounds(0), 
			reloading(false), trigger(false), target_camera_to_shake(nullptr) {}

	private:
		friend class gun_system;

		augmentations::util::timer shooting_timer;
	};
}
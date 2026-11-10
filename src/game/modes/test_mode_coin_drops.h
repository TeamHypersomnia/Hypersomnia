#pragma once
#include "game/cosmos/cosmos.h"
#include "game/components/sentience_component.h"

inline void keep_coins_off_bodies_unless_spawning_money(const bool spawn_money, cosmos& cosm) {
	if (spawn_money) {
		return;
	}

	cosm.for_each_having<components::sentience>([](const auto& typed_handle) {
		typed_handle.template get<components::sentience>().coins_on_body = 0;
	});
}

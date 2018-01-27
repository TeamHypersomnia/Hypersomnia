#include "ingredients.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "game/enums/item_category.h"
#include "game/components/container_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/item_component.h"
#include "game/components/item_slot_transfers_component.h"
#include "game/detail/inventory/perform_transfer.h"

namespace ingredients {
	void add_character_head_inventory(const logic_step step, entity_handle e) {
		auto& item_slot_transfers = e += components::item_slot_transfers();
	}
}
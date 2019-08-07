#include "game/inferred_caches/relational_cache.h"
#include "game/components/fixtures_component.h"
#include "game/components/motor_joint_component.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/for_each_entity.h"
#include "game/inferred_caches/relational_cache.hpp"

#include "augs/templates/enum_introspect.h"

void relational_cache::infer_cache_for(const entity_handle& handle) {
	handle.dispatch_on_having_all<entity_types_passing<concerned_with>>([this](const auto& typed_handle) {
		specific_infer_cache_for(typed_handle);
	});
}

void relational_cache::infer_all(cosmos& cosm) {
	cosm.for_each_entity<concerned_with>([this](const auto& typed_handle) {
		specific_infer_cache_for(typed_handle);
	});
}

void relational_cache::destroy_cache_of(const entity_handle& handle) {
	auto& cosm = handle.get_cosmos();

	handle.dispatch_on_having_all<components::item>([&cosm](const auto& typed_handle) {
		const auto& item = typed_handle.template get<components::item>();
		const auto slot = cosm[item->get_current_slot()];

		if (slot) {
			unset_parenthood(slot, typed_handle);
		}
	});
}
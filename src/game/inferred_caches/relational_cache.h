#pragma once
#include "game/cosmos/entity_id.h"
#include "game/cosmos/entity_handle_declaration.h"

#include "game/detail/inventory/inventory_slot_id.h"

#include "game/cosmos/entity_type_traits.h"
#include "game/inferred_caches/relational_cache_data.h"

class cosmos;

class relational_cache {
public:
	template <class T>
	struct concerned_with {
		static constexpr bool value = has_all_of_v<T, components::item>;
	};

	void infer_all(cosmos&);

	template <class E>
	void specific_infer_cache_for(const E&);

	void infer_cache_for(const entity_handle&);
	void destroy_cache_of(const entity_handle&);
};
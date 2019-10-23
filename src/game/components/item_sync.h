#pragma once
#include "game/cosmos/component_synchronizer.h"
#include "game/components/item_component.h"

#include "game/detail/inventory/item_slot_transfer_request_declaration.h"
#include "game/cosmos/cosmos_solvable_inferred_access.h"
#include "game/detail/inventory/perform_transfer_result.h"

struct perform_transfer_impl {
	perform_transfer_result operator()( 
		const item_slot_transfer_request r, 
		cosmos& cosm
	) const;
};

template <class E>
class component_synchronizer<E, components::item> : public synchronizer_base<E, components::item> {
protected:
	using base = synchronizer_base<E, components::item>;
	using base::operator->;

	template <class H>
	friend void arena_mode_set_transferred_item_meta(H, int charges, const item_owner_meta&);

public:
	using base::get_raw_component;
	using base::base;

	auto get_current_slot() const {
		return get_raw_component().current_slot;
	}

	auto get_charges() const {
		return get_raw_component().charges;
	}

	void set_charges(const int charges) const {
		// TODO: synchronize it somehow.
		this->component->charges = charges;
	}

	auto* operator->() const {
		return this;
	}

	auto& get_owner_meta() const {
		return this->component->owner_meta;
	}
};

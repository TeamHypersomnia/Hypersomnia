#pragma once
#include "game/cosmos/component_synchronizer.h"
#include "game/components/item_component.h"

#include "game/detail/inventory/item_slot_transfer_request_declaration.h"
#include "game/cosmos/cosmos_solvable_inferred_access.h"
#include "game/detail/inventory/perform_transfer_result.h"

perform_transfer_result perform_transfer_impl(
	write_synchronized_component_access,
	cosmos_solvable_inferred_access,
	const item_slot_transfer_request r, 
	cosmos& cosm
); 

template <class E>
class component_synchronizer<E, components::item> : public synchronizer_base<E, components::item> {
protected:
	using base = synchronizer_base<E, components::item>;
	using base::operator->;
public:
	using base::get_raw_component;
	using base::base;

	template <class... Args>
	decltype(auto) perform_transfer(Args&&... args) const {
		return ::perform_transfer_impl(
			write_synchronized_component_access(),
		   	cosmos_solvable_inferred_access(),
		   	std::forward<Args>(args)...
		);
	};

	auto get_current_slot() const {
		const auto& cosm = this->get_handle().get_cosmos();
		return cosm.get_solvable().deguidize(get_raw_component().current_slot);
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
};

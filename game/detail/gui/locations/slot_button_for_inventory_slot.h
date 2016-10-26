#pragma once

class slot_button_for_inventory_slot {
public:
	inventory_slot_id slot_id;

	template <class C>
	bool alive(C context) const {
		return context.get_step().get_cosmos()[slot_id].alive();
	}

	template <class C, class L>
	decltype(auto) polymorphic_call(C context, L lambda) const {
		return lambda(context.get_step().get_cosmos()[slot_id]->button);
	}
};
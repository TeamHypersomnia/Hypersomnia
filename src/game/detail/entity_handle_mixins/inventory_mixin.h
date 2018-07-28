#pragma once
#include <optional>

#include "augs/templates/maybe_const.h"
#include "augs/templates/container_templates.h"
#include "game/detail/inventory/inventory_slot_handle_declaration.h"
#include "game/cosmos/entity_handle_declaration.h"
#include "game/cosmos/step_declaration.h"
#include "game/cosmos/entity_id.h"
#include "augs/build_settings/platform_defines.h"
#include "game/detail/inventory/wielding_result.h"
#include "augs/enums/callback_result.h"

struct attachment_offset_settings;

template <class derived_handle_type>
class inventory_mixin {
	using offset_vector = std::vector<transformr>;

public:
	static constexpr bool is_const = is_handle_const_v<derived_handle_type>;

	using generic_handle_type = basic_entity_handle<is_const>;
	using inventory_slot_handle_type = basic_inventory_slot_handle<generic_handle_type>;

	static constexpr size_t hand_count = 2;
	using hand_selections_array = std::array<entity_id, hand_count>;

	template <class C>
	static bool is_akimbo(const C& cosm, const hand_selections_array& sels) {
		for (const auto& s : sels) {
			if (cosm[s].dead()) {
				return false;
			}
		}

		return true;
	}

	void infer_item_colliders_recursive() const {
		const auto& self = *static_cast<const derived_handle_type*>(this);
		ensure(self);

		self.infer_colliders();
		self.for_each_contained_item_recursive([](const auto h){
			h.infer_colliders();	
			return recursive_callback_result::CONTINUE_AND_RECURSE;
		});
	}

	void infer_change_of_current_slot() const {
		/* Synonym */
		infer_item_colliders_recursive();
	}

	std::optional<unsigned> find_space_occupied() const;	
	int num_charges_fitting_in(const inventory_slot_handle_type&) const;
	void set_charges(int) const;

	generic_handle_type get_owning_transfer_capability() const;
	generic_handle_type get_topmost_container() const;

	bool owning_transfer_capability_alive_and_same_as_of(const entity_id) const;

	std::optional<colliders_connection> calc_connection_to_topmost_container() const;
	std::optional<colliders_connection> calc_connection_until_container(const entity_id until) const;

	template <class handle_type>
	inventory_slot_handle_type determine_holstering_slot_for(const handle_type holstered_item) const;

	template <class handle_type>
	inventory_slot_handle_type determine_pickup_target_slot_for(const handle_type picked_item) const;
	
	inventory_slot_handle_type get_current_slot() const;

	inventory_slot_handle_type get_first_free_hand() const;
	
	inventory_slot_handle_type get_primary_hand() const;
	inventory_slot_handle_type get_secondary_hand() const;

	inventory_slot_handle_type get_hand_no(const size_t) const;
	generic_handle_type get_if_any_item_in_hand_no(const size_t) const;

	template <class F>
	void for_each_hand(F callback) const;

	augs::constant_size_vector<entity_id, 2> get_wielded_guns() const;
	augs::constant_size_vector<entity_id, 2> get_wielded_items() const;

	inventory_item_address get_address_from_root(const entity_id until = entity_id()) const;

	wielding_result make_wielding_transfers_for(const hand_selections_array) const;

	wielding_result swap_wielded_items() const;

private:
	template <class S, class I>
	callback_result for_each_contained_slot_and_item_recursive(
		S slot_callback, 
		I item_callback
	) const;

public:

	template <class A, class G>
	void for_each_attachment_recursive(
		A attachment_callback,
		G get_offsets_by_torso,
		const attachment_offset_settings& settings
	) const;

	template <class I>
	void for_each_contained_item_recursive(I&& item_callback) const;

	template <class S>
	void for_each_contained_slot_recursive(S&& slot_callback) const;

	template <class G>
	ltrb calc_attachments_aabb(G&& get_offsets_by_torso) const;
};

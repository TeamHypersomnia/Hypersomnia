#pragma once
#include <optional>

#include "augs/templates/maybe_const.h"
#include "augs/templates/container_templates.h"
#include "game/detail/inventory/inventory_slot_handle_declaration.h"
#include "game/cosmos/entity_handle_declaration.h"
#include "game/cosmos/step_declaration.h"
#include "game/cosmos/entity_id.h"
#include "augs/build_settings/compiler_defines.h"
#include "game/detail/inventory/wielding_result.h"
#include "augs/enums/callback_result.h"
#include "game/detail/inventory/inventory_space_type.h"
#include "game/organization/special_flavour_id_types.h"
#include "game/detail/inventory/wielding_setup.h"
#include "game/enums/weapon_action_type.h"

struct attachment_offset_settings;
struct colliders_connection;

struct hand_action {
	const std::size_t hand_index;
	const entity_id held_item;
	const weapon_action_type type;
};

enum class wielding_type {
	NOT_WIELDED,
	SINGLE_WIELDED,
	DUAL_WIELDED
};

enum class slot_finding_opt {
	CHECK_WEARABLES,
	CHECK_HANDS,
	CHECK_CONTAINERS,

	COUNT
};

struct pending_item_mount;

using slot_finding_opts = augs::constant_size_vector<slot_finding_opt, 3>;

template <class derived_handle_type>
class inventory_mixin {
	using offset_vector = std::vector<transformr>;

public:
	static constexpr bool is_const = is_handle_const_v<derived_handle_type>;

	using generic_handle_type = basic_entity_handle<is_const>;
	using inventory_slot_handle_type = basic_inventory_slot_handle<generic_handle_type>;

	void infer_item_physics_recursive() const;
	void infer_change_of_current_slot() const;

	std::optional<inventory_space_type> find_space_occupied() const;	
	int num_charges_fitting_in(const inventory_slot_handle_type&) const;
	int count_contained(const item_flavour_id&) const;
	void set_charges(int) const;

	generic_handle_type get_owning_transfer_capability() const;
	generic_handle_type get_topmost_container() const;

	bool owning_transfer_capability_alive_and_same_as_of(const entity_id) const;

	std::optional<colliders_connection> calc_connection_to_topmost_container() const;
	std::optional<colliders_connection> calc_connection_until_container(const entity_id until) const;

	template <class F>
	void for_each_candidate_slot(
		const slot_finding_opts&,
		F&& callback
	) const;

	template <class handle_type>
	inventory_slot_handle_type find_slot_for(
		const handle_type picked_item,
	   	const slot_finding_opts&
	) const;

	template <class handle_type>
	inventory_slot_handle_type find_holstering_slot_for(const handle_type holstered_item) const;

	template <class handle_type>
	inventory_slot_handle_type find_pickup_target_slot_for(const handle_type picked_item) const;
	
	inventory_slot_handle_type get_current_slot() const;

	inventory_slot_handle_type get_first_free_hand() const;
	
	inventory_slot_handle_type get_primary_hand() const;
	inventory_slot_handle_type get_secondary_hand() const;

	inventory_slot_handle_type get_hand_no(std::size_t) const;
	generic_handle_type get_if_any_item_in_hand_no(std::size_t) const;
	hand_action calc_hand_action(std::size_t requested_index) const;
	bool only_secondary_holds_item() const;

	template <class handle_type>
	wielding_type get_wielding_of(const handle_type holstered_item) const;

	template <class F>
	void for_each_hand(F callback) const;

	augs::constant_size_vector<entity_id, 2> get_wielded_guns() const;
	augs::constant_size_vector<entity_id, 2> get_wielded_items() const;
	generic_handle_type get_wielded_other_than(entity_id) const;

	inventory_item_address get_address_from_root(const entity_id until = entity_id()) const;

	template <class S, class I>
	callback_result for_each_contained_slot_and_item_recursive(
		S&& slot_callback, 
		I&& item_callback,
		const optional_slot_flags& filter
	) const;

	template <class A, class G>
	void for_each_attachment_recursive(
		A attachment_callback,
		G get_offsets_by_torso,
		const attachment_offset_settings& settings,
		const bool reverse_hand_order = false
	) const;

	template <class I>
	void for_each_contained_item_recursive(I&& item_callback, const optional_slot_flags& filter = std::nullopt) const;

	template <class S>
	void for_each_contained_slot_recursive(S&& slot_callback, const optional_slot_flags& filter = std::nullopt) const;

	template <class G>
	ltrb calc_attachments_aabb(G&& get_offsets_by_torso) const;

	auto find_mounting_progress() const;
};

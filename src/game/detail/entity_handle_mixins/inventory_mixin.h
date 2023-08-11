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
#include "game/detail/inventory/inventory_slot_types.h"
#include "game/organization/special_flavour_id_types.h"
#include "game/detail/inventory/wielding_setup.h"
#include "game/enums/weapon_action_type.h"
#include "game/detail/inventory/attachment_offset.h"

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

enum class candidate_holster_type {
	WEARABLES,
	HANDS,
	CONTAINERS,

	COUNT
};

enum class slot_finding_opt {
	ALL_CHARGES_MUST_FIT,
	OMIT_MOUNTED_SLOTS,
	COUNT
};

using slot_finding_opts = augs::enum_boolset<slot_finding_opt>;

struct pending_item_mount;

using candidate_holster_types = augs::constant_size_vector<candidate_holster_type, 3>;

template <class derived_handle_type>
class inventory_mixin {
	using offset_vector = std::vector<attachment_offset>;

public:
	static constexpr bool is_const = is_handle_const_v<derived_handle_type>;

	using generic_handle_type = basic_entity_handle<is_const>;
	using inventory_slot_handle_type = basic_inventory_slot_handle<generic_handle_type>;

	void infer_item_physics_recursive() const;
	void infer_change_of_current_slot() const;

	std::optional<inventory_space_type> find_space_occupied() const;	
	int num_charges_fitting_in(const inventory_slot_handle_type&) const;
	int count_contained(const item_flavour_id&) const;

	template <class F>
	int count_contained(F predicate) const;

	void set_charges(int) const;
	int get_charges() const;

	generic_handle_type get_owning_transfer_capability() const;
	generic_handle_type get_topmost_container() const;

	bool owning_transfer_capability_alive_and_same_as_of(const entity_id) const;

	colliders_connection calc_connection_to_topmost_container() const;

	template <class F>
	void for_each_candidate_slot(
		const candidate_holster_types&,
		F&& callback
	) const;

	template <class handle_type>
	inventory_slot_handle_type find_slot_for(
		const handle_type picked_item,
	   	const candidate_holster_types&,
		const slot_finding_opts& opts
	) const;

	template <class handle_type>
	inventory_slot_handle_type find_holstering_slot_for(const handle_type holstered_item) const;

	template <class handle_type>
	inventory_slot_handle_type find_pickup_target_slot_for(const handle_type picked_item, slot_finding_opts = slot_finding_opts()) const;
	
	inventory_slot_handle_type get_current_slot() const;

	inventory_slot_handle_type get_first_free_hand() const;
	
	inventory_slot_handle_type get_primary_hand() const;
	inventory_slot_handle_type get_secondary_hand() const;

	inventory_slot_handle_type get_hand_no(std::size_t) const;
	generic_handle_type get_if_any_item_in_hand_no(std::size_t) const;

	hand_action calc_hand_action(std::size_t requested_index) const;
	hand_action calc_viable_hand_action(std::size_t requested_index) const;

	bool only_secondary_holds_item() const;

	template <class handle_type>
	wielding_type get_wielding_of(const handle_type holstered_item) const;

	template <class F>
	void for_each_hand(F callback) const;

	augs::constant_size_vector<entity_id, 2> get_wielded_guns() const;
	augs::constant_size_vector<entity_id, 2> get_wielded_items() const;
	augs::constant_size_vector<entity_id, 2> get_wielded_melees() const;
	generic_handle_type get_wielded_other_than(entity_id) const;

	inventory_item_address get_address_from_root(const entity_id until = entity_id()) const;

	template <class S, class I>
	callback_result for_each_contained_slot_and_item_recursive(
		S&& slot_callback, 
		I&& item_callback,
		const optional_slot_flags& filter
	) const;

	template <class A>
	void with_each_attachment_recursive(
		A attachment_callback,
		const attachment_offset_settings& settings,
		attachment_offset initial_offset = attachment_offset()
	) const;

	template <class A, class B, class G>
	void recurse_character_attachments(
		A attachment_callback,
		B should_recurse,
		G get_offsets_by_torso,
		const attachment_offset_settings& settings,
		const bool flip_hands_order
	) const;

	template <class I>
	void for_each_contained_item_recursive(I&& item_callback, const optional_slot_flags& filter = std::nullopt) const;

	template <class S>
	void for_each_contained_slot_recursive(S&& slot_callback, const optional_slot_flags& filter = std::nullopt) const;

	ltrb calc_aabb_with_attachments() const;

	auto find_mounting_progress() const;
};

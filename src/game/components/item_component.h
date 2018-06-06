#pragma once
#include <optional>

#include "game/components/transform_component.h"

#include "game/messages/queue_destruction.h"
#include "game/messages/item_picked_up_message.h"
#include "game/messages/interpolation_correction_request.h"

#include "game/detail/inventory/inventory_slot_id.h"

#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/component_synchronizer.h"
#include "game/transcendental/cosmos_solvable_inferred_access.h"

#include "game/detail/inventory/inventory_slot_handle_declaration.h"
#include "game/detail/inventory/item_slot_transfer_request_declaration.h"

#include "game/enums/item_category.h"
#include "game/enums/item_holding_stance.h"

#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/view_input/particle_effect_input.h"

struct perform_transfer_result {
	std::optional<messages::queue_destruction> destructed;
	std::vector<messages::interpolation_correction_request> interpolation_corrected;
	std::optional<messages::item_picked_up_message> picked;

	std::optional<packaged_sound_effect> transfer_sound;
	std::optional<packaged_particle_effect> transfer_particles;

	void notify(logic_step) const;
	void play_effects(logic_step) const;
};

perform_transfer_result perform_transfer(
	write_synchronized_component_access,
	cosmos_solvable_inferred_access,
	const item_slot_transfer_request r, 
	cosmos& cosmos
); 

namespace components {
	struct item {
		static constexpr bool is_synchronized = true;

		// GEN INTROSPECTOR struct components::item
		int charges = 1;
		signi_inventory_slot_id current_slot;
		// END GEN INTROSPECTOR

#if TODO_MOUNTING
		enum mounting_state : unsigned char {
			MOUNTED,
			UNMOUNTED,
		};

		inventory_slot_id target_slot_after_unmount;

		mounting_state current_mounting = UNMOUNTED;
		mounting_state intended_mounting = UNMOUNTED;

		void set_mounted();
		void request_mount();
		void cancel_montage();
		void request_unmount();
		void request_unmount(inventory_slot_id target_slot_after_unmount);
		void reset_mounting_timer();

		bool is_mounted() const;
#endif
	};
}

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
		return ::perform_transfer(
			write_synchronized_component_access(),
		   	cosmos_solvable_inferred_access(),
		   	std::forward<Args>(args)...
		);
	};

	auto get_current_slot() const {
		const auto& cosmos = this->get_handle().get_cosmos();
		return cosmos.get_solvable().deguidize(get_raw_component().current_slot);
	}

	auto get_charges() const {
		return get_raw_component().charges;
	}

	void set_charges(const int charges) const {
		// TODO: synchronize it somehow? For now this function is only used by the test scenes,
		// but might later be needed by some spells - or maybe these should just do a perform_transfer?
		this->component->charges = charges;
	}

	auto* operator->() const {
		return this;
	}
};

namespace invariants {
	struct item {
		static constexpr bool reinfer_when_tweaking = true;
		// GEN INTROSPECTOR struct invariants::item
		bool stackable = false;
		pad_bytes<3> pad;

		item_holding_stance holding_stance = item_holding_stance::PISTOL_LIKE;

		unsigned space_occupied_per_charge = 1;

		unsigned dual_wield_accuracy_loss_percentage = 50;
		unsigned dual_wield_accuracy_loss_multiplier = 1;

		sound_effect_input wield_sound;
		sound_effect_input wear_sound;

		item_category_flagset categories_for_slot_compatibility = { item_category::GENERAL };
		// END GEN INTROSPECTOR
	};
}
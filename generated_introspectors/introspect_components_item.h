#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(current_mounting));
		f(t.NVP(intended_mounting));

		f(t.NVP(categories_for_slot_compatibility));

		f(t.NVP(charges));
		f(t.NVP(space_occupied_per_charge));
		f(t.NVP(stackable));

		f(t.NVP(dual_wield_accuracy_loss_percentage));
		f(t.NVP(dual_wield_accuracy_loss_multiplier));

		f(t.NVP(current_slot));
		f(t.NVP(target_slot_after_unmount));

		f(t.NVP(montage_time_ms));
		f(t.NVP(montage_time_left_ms));
	}

}
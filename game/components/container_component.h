#pragma once
#include "game/enums/slot_function.h"
#include "game/detail/inventory_slot.h"

#include "augs/misc/enum_associative_array.h"

namespace components {
	struct container {
		augs::enum_associative_array<slot_function, inventory_slot> slots;

		template<class F>
		void for_each_held_id(F f) {
			for (auto& s : slots) {
				for (auto& i : s.second.items_inside) {
					f(i);
				}
			}
		}

		template<class F>
		void for_each_held_id(F f) const {
			for (const auto& s : slots) {
				for (const auto& i : s.second.items_inside) {
					f(i);
				}
			}
		}

		template <class Archive>
		void serialize(Archive& ar) {
			ar(CEREAL_NVP(slots));
		}
	};
}

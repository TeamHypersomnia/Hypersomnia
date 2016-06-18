#include "damage_component.h"

namespace components {
	bool damage::can_merge_entities(const entity_id& e1, const entity_id& e2) {
		auto* pa = e1->find<damage>();
		auto* pb = e2->find<damage>();
		if (!pa && !pb) return true;
		if (!(pa && pb)) return false;

		auto& a = *pa;
		auto& b = *pb;
		
		return
		std::make_tuple(a.amount, a.damage_upon_collision, a.destroy_upon_damage, a.constrain_lifetime, a.constrain_distance, a.max_distance, a.max_lifetime_ms) ==
		std::make_tuple(b.amount, b.damage_upon_collision, b.destroy_upon_damage, b.constrain_lifetime, b.constrain_distance, b.max_distance, b.max_lifetime_ms);
	}
}
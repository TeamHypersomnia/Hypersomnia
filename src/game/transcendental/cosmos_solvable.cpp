#include "game/transcendental/cosmos_solvable.h"
#include "game/organization/for_each_component_type.h"

#include "augs/templates/introspect.h"

const cosmos_solvable cosmos_solvable::zero;

void cosmos_solvable::clear() {
	*this = zero;
}

bool cosmos_solvable::empty() const {
	return get_entities_count() == 0 && guid_to_id.empty();
}

cosmos_solvable::cosmos_solvable(const cosmic_pool_size_type reserved_entities) {
	reserve_storage_for_entities(reserved_entities);
}

static auto make_reserver(const std::size_t n) {
	return [n](auto, auto& sys) {
		using T = std::decay_t<decltype(sys)>;

		if constexpr(can_reserve_caches_v<T>) {
			sys.reserve_caches_for_entities(n);
		}
	};
}

void cosmos_solvable::reserve_storage_for_entities(const cosmic_pool_size_type n) {
	get_entity_pool().reserve(n);

	for_each_through_std_get(
		significant.component_pools, 
		[n](auto& p){
			p.reserve(n);
		}
	);

	augs::introspect(make_reserver(n), inferred);
}

void cosmos_solvable::destroy_all_caches() {
	inferred.~cosmos_solvable_inferred();
	new (&inferred) cosmos_solvable_inferred;

	const auto n = significant.entity_pool.capacity();

	augs::introspect(make_reserver(n), inferred);
}

void cosmos_solvable::increment_step() {
	++significant.clock.now.step;
}

template <class T, class agg>
static bool components_equal_in_entities(
	const cosmos_solvable& provider,
	const agg& e1, 
	const agg& e2
) {
	const auto maybe_1 = e1.template find<T>(provider);
	const auto maybe_2 = e2.template find<T>(provider);

	if (!maybe_1 && !maybe_2) {
		return true;
	}

	if (maybe_1) {
		if (maybe_2) {
			bool difference_found = false;

			if (!augs::equal_by_introspection(*maybe_1, *maybe_2)) {
				difference_found = true;
			}

			if (!difference_found) {
				return true;
			}
		}
	}

	return false;
};

bool cosmos_solvable::operator==(const cosmos_solvable& b) const {
	ensure(guid_to_id.size() == get_entities_count());
	ensure(b.guid_to_id.size() == b.get_entities_count());

	if (get_entities_count() != b.get_entities_count()) {
		return false;
	}

	for (const auto& it : guid_to_id) {
		const auto guid = it.first;

		const auto& left = get_aggregate(it.second);

		const auto b_id = b.get_entity_id_by(guid);

		if (!b_id.is_set()) {
			return false;
		}

		const auto& right = b.get_aggregate(b_id);

		bool difference_found = false;

		for_each_component_type([&](auto c) {
			if (!components_equal_in_entities<decltype(c)>(*this, left, right)) {
				difference_found = true;
			}
		});

		if (difference_found) {
			return false;
		}
	}

	return true;
}

bool cosmos_solvable::operator!=(const cosmos_solvable& b) const {
	return !operator==(b);
}

double cosmos_solvable::get_total_seconds_passed(const double view_interpolation_ratio) const {
	return get_total_seconds_passed() + get_fixed_delta().per_second(view_interpolation_ratio);
}

double cosmos_solvable::get_total_seconds_passed() const {
	return significant.clock.now.step * get_fixed_delta().in_seconds<double>();
}

decltype(augs::stepped_timestamp::step) cosmos_solvable::get_total_steps_passed() const {
	return significant.clock.now.step;
}

augs::stepped_timestamp cosmos_solvable::get_timestamp() const {
	return significant.clock.now;
}

augs::delta cosmos_solvable::get_fixed_delta() const {
	return significant.clock.delta;
}

void cosmos_solvable::set_steps_per_second(const unsigned steps) {
	significant.clock.delta = augs::delta::steps_per_second(steps);
}

unsigned cosmos_solvable::get_steps_per_second() const {
	return get_fixed_delta().in_steps_per_second();
}

entity_id cosmos_solvable::allocate_new_entity() {
	if (get_entity_pool().full()) {
		throw std::runtime_error("Entities should be controllably reserved to avoid invalidation of entity_handles.");
	}

	return get_entity_pool().allocate();
}

entity_id cosmos_solvable::allocate_entity_with_specific_guid(const entity_guid specific_guid) {
	const auto id = allocate_new_entity();
	get_aggregate(id).get<components::guid>(*this).value = specific_guid;
	guid_to_id[specific_guid] = id;

	return id;
}

entity_id cosmos_solvable::allocate_next_entity() {
	const auto next_guid = significant.clock.next_entity_guid.value++;
	return allocate_entity_with_specific_guid(next_guid);
}

void cosmos_solvable::free_entity(const entity_id id) {
	clear_guid(id);
	get_entity_pool().free(id);
}

void cosmos_solvable::clear_guid(const entity_id cleared) {
	guid_to_id.erase(get_guid(cleared));
}


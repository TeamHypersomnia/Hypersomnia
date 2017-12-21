#include "game/transcendental/cosmos_solvable_state.h"

const cosmos_solvable_state cosmos_solvable_state::zero;

void cosmos_solvable_state::clear() {
	*this = zero;
}

cosmos_solvable_state::cosmos(const cosmic_pool_size_type reserved_entities) : solvable(reserved_entities) {
	reserve_storage_for_entities(reserved_entities);
}

void cosmos_solvable_state::reserve_storage_for_entities(const cosmic_pool_size_type n) {
	get_entity_pool().reserve(n);
	reserve_all_components(n);

	inferred.for_each([n](auto& sys) {
		sys.reserve_caches_for_entities(n);
	});
}

void cosmos_solvable_state::destroy_all_caches() {
	inferred.~all_inferred_caches();
	new (&inferred) all_inferred_caches;

	const auto n = significant.entity_pool.capacity();

	inferred.for_each([n](auto& sys) {
		sys.reserve_caches_for_entities(n);
	});
}

void cosmos_solvable_state::increment_step() {
	++significant.meta.now.step;
}

template <class T, class agg>
static bool components_equal_in_entities(
	const T& provider,
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

bool cosmos_solvable_state::operator==(const cosmos_solvable_state& b) const {
	ensure(guid_to_id.size() == get_entities_count());
	ensure(b.guid_to_id.size() == b.get_entities_count());

	if (get_entities_count() != b.get_entities_count()) {
		return false;
	}

	for (const auto& it : guid_to_id) {
		const auto guid = it.first;

		const auto& left = get_agg(it.second);

		if (!b.entity_exists_by(guid)) {
			return false;
		}

		const auto& right = b.get_agg(guid);

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

bool cosmos_solvable_state::operator!=(const cosmos& b) const {
	return !operator==(b);
}

double cosmos_solvable_state::get_total_seconds_passed(const double view_interpolation_ratio) const {
	return get_total_seconds_passed() + get_fixed_delta().per_second(view_interpolation_ratio);
}

double cosmos_solvable_state::get_total_seconds_passed() const {
	return significant.meta.now.step * get_fixed_delta().in_seconds<double>();
}

decltype(augs::stepped_timestamp::step) cosmos_solvable_state::get_total_steps_passed() const {
	return significant.meta.now.step;
}

augs::stepped_timestamp cosmos_solvable_state::get_timestamp() const {
	return significant.meta.now;
}

augs::delta cosmos_solvable_state::get_fixed_delta() const {
	return significant.meta.delta;
}

void cosmos_solvable_state::set_steps_per_second(const unsigned steps) {
	significant.meta.delta = augs::delta::steps_per_second(steps);
}

unsigned cosmos_solvable_state::get_steps_per_second() const {
	return get_fixed_delta().in_steps_per_second();
}

entity_id cosmos_solvable_state::allocate_new_entity() {
	if (get_entity_pool().full()) {
		throw std::runtime_error("Entities should be controllably reserved to avoid invalidation of entity_handles.");
	}

	return get_entity_pool().allocate();
}

entity_id cosmos_solvable_state::allocate_entity_with_specific_guid(const entity_guid specific_guid) {
	const auto id = allocate_new_entity();
	get_agg(id).get<components::guid>(*this).value = specific_guid;
	guid_to_id[specific_guid] = id;

	return id;
}

entity_id cosmos_solvable_state::allocate_next_entity() {
	const auto next_guid = s.significant.meta.next_entity_guid.value++;
	return allocate_entity_with_specific_guid(next_guid);
}

void cosmos_solvable_state::clear_guid(const entity_id cleared) {
	guid_to_id.erase(get_guid(cleared));
}

void cosmos_solvable_state::remap_guids() {
	guid_to_id.clear();

	for_each_entity_id([this](const entity_id id) {
		guid_to_id[get_guid(id)] = id;
	});
}

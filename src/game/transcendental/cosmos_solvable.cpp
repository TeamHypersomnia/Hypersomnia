#include "game/transcendental/cosmos_solvable.h"

#include "augs/templates/introspect.h"

const cosmos_solvable cosmos_solvable::zero;

void cosmos_solvable::clear() {
	*this = zero;
}

bool cosmos_solvable::empty() const {
	return get_entities_count() == 0;
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
	for_each_pool([n](auto& p){
		p.reserve(n);
	});

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

void cosmos_solvable::free_entity(const entity_id id) {
	clear_guid(id);

	significant.on_pool(id.type_id, [id](auto& p){ p.free(id); });
}

void cosmos_solvable::clear_guid(const entity_id cleared) {
	guid_to_id.erase(get_guid(cleared));
}


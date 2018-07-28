#pragma once
#include "game/cosmos/component_synchronizer.h"

template <class E>
class component_synchronizer<E, components::overridden_size> : public synchronizer_base<E, components::overridden_size> {
protected:
	using base = synchronizer_base<E, components::overridden_size>;
	using base::operator->;
	using base::handle;

	void infer_caches() const {
		handle.get_cosmos().get_solvable_inferred({}).tree_of_npo.infer_cache_for(handle);
		handle.infer_colliders_from_scratch();
	}

public:
	using base::base;

	const auto& get() const {
		return this->component->size;
	}

	void set(const vec2 new_size) const {
		this->component->size.emplace(new_size);
		infer_caches();
	}

	void reset() const {
		this->component->size.is_enabled = false;
		infer_caches();
	}
};
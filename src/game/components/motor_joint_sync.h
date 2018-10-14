#pragma once
#include "game/cosmos/component_synchronizer.h"

template <class E>
class component_synchronizer<E, components::motor_joint> : public synchronizer_base<E, components::motor_joint> {
protected:
	friend class ::physics_world_cache;

	using base = synchronizer_base<E, components::motor_joint>;
	using base::handle;
public:
	void infer_caches() const {
		const auto h = handle.to_const_generic();

		handle.get_cosmos().get_solvable_inferred({}).relational.infer_cache_for(h);
		handle.get_cosmos().get_solvable_inferred({}).physics.infer_cache_for(h);
	}

	using base::get_raw_component;
	using base::base;

	bool is_activated() const{
		return get_raw_component().activated;
	}

	auto get_target_bodies() const {
		return get_raw_component().target_bodies;
	}

	auto& operator=(const components::motor_joint& m) const {
		get_raw_component({}) = m;
		infer_caches();
		return *this;
	}
};


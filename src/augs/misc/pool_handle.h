#pragma once
#include "augs/templates/maybe_const.h"
#include "augs/misc/pooled_object_id.h"

namespace augs {
	template<bool is_const, class pool_container, class value_type>
	class handle_for_pool_container {
	public:
		using owner_reference = maybe_const_ref_t<is_const, pool_container>;
		using id_type = pooled_object_id<value_type>;

		owner_reference owner;
		id_type raw_id;
		
		handle_for_pool_container(
			owner_reference owner, 
			const id_type raw_id
		) : 
			raw_id(raw_id), 
			owner(owner) 
		{}

		decltype(auto) get() const {
			return owner.get(raw_id);
		}

		bool alive() const {
			return owner.alive(raw_id);
		}

		bool dead() const {
			return !alive();
		}

		operator id_type() const {
			return raw_id;
		}

		owner_reference get_pool() {
			return owner;
		}
	};
}

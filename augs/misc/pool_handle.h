#pragma once
#include "templates.h"
#include "pool_id.h"

namespace augs {
	template<bool is_const, class owner_type, class value_type>
	class basic_handle_base {
	public:
		typedef maybe_const_ref_t<is_const, owner_type> owner_reference;
		typedef maybe_const_ref_t<is_const, value_type> value_reference;
		typedef pool_id<value_type> id_type;

		owner_reference owner;
		id_type raw_id;

		basic_handle_base(owner_reference owner, id_type raw_id) : owner(owner), raw_id(raw_id) {}

		void unset() {
			raw_id.unset();
		}
		
		void set_id(id_type id) {
			raw_id = id;
		}

		void set_debug_name(std::string s) {
			raw_id.set_debug_name(s);
		}

		decltype(auto) get_pool() {
			return owner.get_pool(id_type());
		}

		decltype(auto) get_pool() const {
			return owner.get_pool(id_type());
		}

		value_reference get() const {
			return get_pool().get(raw_id);
		}

		bool alive() const {
			return get_pool().alive(raw_id);
		}

		bool dead() const {
			return !alive();
		}

		id_type get_id() const {
			return raw_id;
		}

		operator id_type() const {
			return get_id();
		}

		std::string get_debug_name() const {
			return raw_id.get_debug_name();
		}

		bool operator==(const id_type& b) const {
			return raw_id == b;
		}

		bool operator!=(const id_type& b) const {
			return raw_id != b;
		}
	};

	template<bool is_const, class owner_type, class value_type>
	class basic_handle : public basic_handle_base<is_const, owner_type, value_type> {
	public:
		using basic_handle_base::basic_handle_base;
	};

	template <class T>
	class basic_pool;

	template<bool is_const, class T>
	using basic_pool_handle = basic_handle<is_const, basic_pool<T>, T>;

	template<class T>
	using pool_handle = basic_pool_handle<false, T>;
	
	template<class T>
	using const_pool_handle = basic_pool_handle<true, T>;
}
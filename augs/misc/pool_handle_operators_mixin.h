#pragma once
#include <vector>

struct inventory_slot_id;

namespace augs {
	template<class derived>
	class pool_handle_operators_mixin {
	public:
		template <class container>
		decltype(auto) to_handle_vector(const container& vec) {
			auto& self = *static_cast<derived*>(this);

			std::vector<decltype(operator[](*vec.begin()))> handles;

			for (auto v : vec)
				handles.emplace_back(operator[](v));

			return handles;
		}

		template <class container>
		decltype(auto) to_handle_vector(const container& vec) const {
			auto& self = *static_cast<const derived*>(this);

			std::vector<decltype(operator[](*vec.begin()))> handles;

			for (auto v : vec)
				handles.emplace_back(operator[](v));

			return handles;
		}

		template <class id_container>
		decltype(auto) operator [](const id_container& ids) {
			return to_handle_vector(ids);
		}

		template <class id_container>
		decltype(auto) operator [](const id_container& ids) const {
			return to_handle_vector(ids);
		}

		template <class object_type>
		decltype(auto) operator[](pool_id<object_type> from_id) {
			auto& self = *static_cast<derived*>(this);
			return self.get_handle(from_id);
		}

		template <class object_type>
		decltype(auto) operator[](pool_id<object_type> from_id) const {
			auto& self = *static_cast<const derived*>(this);
			return self.get_handle(from_id);
		}

		template <class object_type>
		decltype(auto) operator[](unversioned_id<object_type> unv) {
			auto& self = *static_cast<derived*>(this);
			return self.get_handle(self.get_pool(pool_id<object_type>()).make_versioned(unv));
		}

		template <class object_type>
		decltype(auto) operator[](unversioned_id<object_type> unv) const {
			auto& self = *static_cast<const derived*>(this);
			return self.get_handle(self.get_pool(pool_id<object_type>()).make_versioned(unv));
		}

		decltype(auto) operator[](inventory_slot_id from_id) {
			auto& self = *static_cast<derived*>(this);
			return self.get_handle(from_id);
		}

		decltype(auto) operator[](inventory_slot_id from_id) const {
			auto& self = *static_cast<const derived*>(this);
			return self.get_handle(from_id);
		}
	};
}

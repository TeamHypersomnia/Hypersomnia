#pragma once
#include <vector>

namespace augs {
	template<class derived>
	class object_pool_handlizer {
	public:
		template <class id_type>
		decltype(auto) to_handle_vector(std::vector<id_type> vec) {
			auto& self = *static_cast<derived*>(this);

			auto dummy = self.get_handle(T());

			std::vector<decltype(dummy)> handles;

			for (auto v : vec)
				handles.emplace_back(self.get_handle(v));

			return std::move(handles);
		}

		template <class id_type>
		decltype(auto) to_handle_vector(std::vector<id_type> vec) const {
			auto& self = *static_cast<const derived*>(this);
			auto dummy = self.get_handle(T());

			std::vector<decltype(dummy)> handles;

			for (auto v : vec)
				handles.emplace_back(self.get_handle(v));

			return std::move(handles);
		}

		template <class id_type>
		decltype(auto) operator[](id_type from_id) {
			auto& self = *static_cast<derived*>(this);
			return self.get_handle(from_id);
		}

		template <class id_type>
		decltype(auto) operator[](id_type from_id) const {
			auto& self = *static_cast<const derived*>(this);
			return self.get_handle(from_id);
		}

		template <class id_type>
		decltype(auto) operator [](std::vector<id_type> ids) {
			return to_handle_vector(ids);
		}

		template <class id_type>
		decltype(auto) operator [](std::vector<id_type> ids) const {
			return to_handle_vector(ids);
		}
	};
}

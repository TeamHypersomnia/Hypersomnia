#pragma once

namespace augs {
	template<class derived>
	class subscript_handle_getters_mixin {
	public:
		template <class id_type>
		decltype(auto) operator[](const id_type id) {
			auto& self = *static_cast<derived*>(this);
			return subscript_handle_getter(self, id);
		}

		template <class id_type>
		decltype(auto) operator[](const id_type id) const {
			const auto& self = *static_cast<const derived*>(this);
			return subscript_handle_getter(self, id);
		}
	};
}

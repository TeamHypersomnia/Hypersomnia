#pragma once

namespace augs {
	template<class derived>
	class subscript_operator_for_get_handle_mixin {
	public:
		template <class id_type>
		decltype(auto) operator[](const id_type id) {
			auto& self = *static_cast<derived*>(this);
			return self.get_handle(id);
		}

		template <class id_type>
		decltype(auto) operator[](const id_type id) const {
			const auto& self = *static_cast<const derived*>(this);
			return self.get_handle(id);
		}
	};
}

#pragma once
#include <vector>

namespace augs {
	template<class derived>
	class subscript_operator_for_get_handle_mixin {
		template <class T, typename = void>
		class container_or_object_resolution {
		public:
			static decltype(auto) handlize(subscript_operator_for_get_handle_mixin& base, const T& from_id) {
				auto& self = *static_cast<derived*>(&base);
				return self.get_handle(from_id);
			}

			static decltype(auto) handlize(const subscript_operator_for_get_handle_mixin& base, const T& from_id) {
				const auto& self = *static_cast<const derived*>(&base);
				return self.get_handle(from_id);
			}
		};

		template <class T>
		class container_or_object_resolution<T, std::enable_if_t<std::is_same<std::true_type, decltype(std::declval<T>().begin(), std::true_type())>::value>> {
		public:
			static decltype(auto) handlize(subscript_operator_for_get_handle_mixin& base, const T& passed_container) {
				auto& self = *static_cast<derived*>(&base);

				std::vector<decltype(self.get_handle(*passed_container.begin()))> handles;

				for (auto v : passed_container)
					handles.emplace_back(self.get_handle(v));

				return handles;
			}

			static decltype(auto) handlize(const subscript_operator_for_get_handle_mixin& base, const T& passed_container) {
				const auto& self = *static_cast<const derived*>(&base);

				std::vector<decltype(self.get_handle(*passed_container.begin()))> handles;

				for (auto v : passed_container)
					handles.emplace_back(self.get_handle(v));

				return handles;
			}
		};

	public:

		template <class passed_type>
		decltype(auto) operator[](const passed_type passed) {
			return container_or_object_resolution<passed_type>::handlize(*this, passed);
		}

		template <class passed_type>
		decltype(auto) operator[](const passed_type passed) const {
			return container_or_object_resolution<passed_type>::handlize(*this, passed);
		}
	};
}

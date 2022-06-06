#pragma once
#include "augs/templates/traits/is_enum_map.h"
#include "augs/templates/get_by_dynamic_id.h"
#include "application/setups/debugger/detail/field_address.h"

template <class F>
auto continue_if_nullopt(F callback) {
	return [callback](auto& resolved) -> callback_result {
		if constexpr(!std::is_same_v<std::nullopt_t, remove_cref<decltype(resolved)>>) {
			return callback(resolved);
		}

		return callback_result::CONTINUE;
	};
}

template <class O, class field_type_id, class F>
decltype(auto) on_field_address(
	O& object,
	const field_address<field_type_id>& address,
	F callback
) {
	static constexpr bool is_const = std::is_const_v<O>;
	using byte_type = maybe_const_ptr_t<is_const, std::byte>;

	auto l = [&](const auto& typed_field) {
		using T = remove_cref<decltype(typed_field)>;
		using location_ptr_type = maybe_const_ptr_t<is_const, T>;

		const auto object_location = reinterpret_cast<byte_type>(std::addressof(object));
		const auto field_location = reinterpret_cast<location_ptr_type>(object_location + address.offset);

		if constexpr(is_enum_map_v<T>) {
			const auto index = address.element_index;

			if (index == static_cast<unsigned>(-1)) {
				return callback(*field_location);
			}
			else if (const auto mapped = mapped_or_nullptr(*field_location, static_cast<typename T::key_type>(index))) {
				return callback(*mapped);
			}
			else {
				return callback(std::nullopt);
			}
		}
		else if constexpr(can_access_data_v<T>) {
			const auto index = address.element_index;

			if (index == static_cast<unsigned>(-1)) {
				return callback(*field_location);
			}
			else if (index < field_location->size()) {
				return callback(field_location->at(index));
			}
			else {
				return callback(std::nullopt);
			}
		}
		else {
			return callback(*field_location);
		}
	};

	return get_by_dynamic_id(
		typename field_type_id::list_type(),
		address.type_id,
		l
	);
}

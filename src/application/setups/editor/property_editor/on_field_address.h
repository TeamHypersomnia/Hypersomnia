#pragma once
#include "augs/templates/get_by_dynamic_id.h"
#include "application/setups/editor/property_editor/property_editor_structs.h"

template <class O, class F>
decltype(auto) on_field_address(
	O& object,
	const field_address& address,
	F callback
) {
	static constexpr bool is_const = std::is_const_v<O>;
	using byte_type = maybe_const_ptr_t<is_const, std::byte>;

	return get_by_dynamic_id(
		edited_field_type_id::list_type(),
		address.type_id,
		[&](const auto& typed_field) {
			using T = std::decay_t<decltype(typed_field)>;
			using location_ptr_type = maybe_const_ptr_t<is_const, T>;

			const auto object_location = reinterpret_cast<byte_type>(std::addressof(object));
			const auto field_location = reinterpret_cast<location_ptr_type>(object_location + address.offset);

			return callback(*field_location);
		}
	);
}

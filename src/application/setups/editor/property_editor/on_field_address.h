#pragma once
#include "augs/templates/get_by_dynamic_id.h"
#include "application/setups/editor/property_editor/property_editor_structs.h"

template <class O, class F>
decltype(auto) on_field_address(
	O& object,
	const field_address& address,
	F callback
) {
	return get_by_dynamic_id(
		edited_field_type_id::list_type(),
		address.type_id,
		[&](const auto& typed_field) {
			using T = std::decay_t<decltype(typed_field)>;

			const auto object_location = reinterpret_cast<std::byte*>(std::addressof(object));
			const auto field_location = reinterpret_cast<T*>(object_location + address.offset);

			return callback(*field_location);
		}
	);
}

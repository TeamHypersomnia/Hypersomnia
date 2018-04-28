#pragma once
#include "augs/string/format_enum.h"
#include "augs/string/typesafe_sprintf.h"
#include "application/setups/editor/property_editor/property_editor_structs.h"

template <class A, class B>
auto describe_changed_flag(
	const A& field_name,
	const B& new_value
) {
	return typesafe_sprintf(
		"%x %x", 
		new_value ? "Set" : "Unset", 
		field_name
	);
};

template <class A, class B>
auto describe_changed_generic(
	const A& field_name,
	const B& new_value
) {
	if constexpr(std::is_enum_v<B>) {
		return typesafe_sprintf(
			"Set %x to %x",
			field_name,
			format_enum(new_value)
		);
	}
	else {
		return typesafe_sprintf(
			"Set %x to %x",
			field_name,
			new_value
		);
	}
};

template <class A, class B>
auto describe_changed(
	const A& field_name,
   	const B& new_value
) {
	using F = remove_cref<decltype(new_value)>;

	if constexpr(std::is_same_v<F, bool>) {
		return describe_changed_flag(field_name, new_value);
	}
	else if constexpr(std::is_same_v<F, std::string>) {
		return describe_changed_generic(field_name, new_value);
	}
	else if constexpr(is_container_v<F>) {
		return typesafe_sprintf("Altered %x", field_name);
	}
	else if constexpr(can_stream_left_v<std::ostringstream, F>) {
		return describe_changed_generic(field_name, new_value);
	}
	else {
		return typesafe_sprintf("Altered %x", field_name);
	}
};

template <class A, class B, class S>
auto describe_changed(
	const A& field_name,
	const B& new_value,
	const S& special_provider
) {
	if constexpr(S::template handles<B>) {
		return special_provider.describe_changed(field_name, new_value);
	}
	else {
		return describe_changed(field_name, new_value);
	}
}


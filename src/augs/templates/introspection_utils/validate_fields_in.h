#pragma once
#include "augs/templates/introspect.h"

#include "augs/templates/traits/container_traits.h"
#include "augs/templates/traits/is_variant.h"

template <class F, class T>
void validate_fields_in(T& object, F condition) {
	condition(object);

	if constexpr(has_value_type_v<T>) {
		typename T::value_type v;
		validate_fields_in(v, condition);
	}
	else if constexpr(is_variant_v<T>) {
		T t;
		for_each_through_std_get(t, [&](auto& f){ validate_fields_in(f, condition); });
	}
	else if constexpr(has_introspect_v<T>){
		augs::introspect([&](auto, auto& m) {
			validate_fields_in(m, condition);
		}, object);
	}
}

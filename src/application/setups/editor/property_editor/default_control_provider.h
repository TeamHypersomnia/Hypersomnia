#pragma once
#include <string>
#include "augs/templates/identity_templates.h"

struct default_control_provider {
	template <class T>
	static constexpr bool handles = always_false_v<T>;

	template <class T>
	bool handle(const std::string& label, const T& object) {
		static_assert(handles<T>());
		return false;
	}

	template <class T>
	std::string describe_changed(const std::string& label, const T& from, const T& to) {
		static_assert(always_false_v<T>);
		return "";
	}
};

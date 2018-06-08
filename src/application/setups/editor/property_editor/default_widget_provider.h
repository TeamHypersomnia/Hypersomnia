#pragma once
#include <string>
#include "augs/templates/identity_templates.h"

struct default_widget_provider {
	template <class... Args>
	default_widget_provider(Args&&...) {}

	template <class T>
	static constexpr bool handles = always_false_v<T>;

	template <class T>
	bool handle(const std::string&, const T&) {
		static_assert(handles<T>());
		return false;
	}

	template <class T>
	bool handle_container_prologue(const std::string&, const T&) {
		static_assert(handles<typename T::value_type>);
		return false;
	}

	template <class T>
	std::string describe_changed(const std::string&, const T&, const T&) {
		static_assert(always_false_v<T>);
		return "";
	}
};

struct default_sane_default_provider {
	template <class T>
	auto construct() const {
		return T();
	}
};

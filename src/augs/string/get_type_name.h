#pragma once
#include <string>

#include "augs/templates/remove_cref.h"
#include "augs/templates/traits/is_std_array.h"

std::string demangle(const char*);

template <class T, class = void>
struct has_custom_type_name : std::false_type {};

template <class T>
struct has_custom_type_name<T, decltype(T::get_custom_type_name(), void())> : std::true_type {};

template <class T>
constexpr bool has_custom_type_name_v = has_custom_type_name<T>::value;

template <class T>
const std::string& get_type_name() {
	static const std::string name = [](){
		if constexpr(is_std_array_v<T>) {
			return get_type_name<typename T::value_type>() + '[' + std::to_string(is_std_array<T>::size) + ']';
		}
		else if constexpr(is_enum_array_v<T>) {
			return get_type_name<typename T::value_type>() + '[' + get_type_name<typename T::enum_type>() + "::COUNT (" + std::to_string(is_enum_array<T>::size) + ")]";
		}
		else if constexpr(std::is_same_v<T, std::string>) {
			return "std::string";
		}
		else {
			return demangle(typeid(T).name());
		}
	}();

	return name;
}

template <class T>
const std::string& get_type_name_strip_namespace() {
	static const std::string name = []() {
		if constexpr(has_custom_type_name_v<T>) {
			return T::get_custom_type_name();
		}
		else {
			auto name = get_type_name<T>();

			while (true) {
				const auto it = name.find("::");

				if (it == std::string::npos) {
					break;
				}

				const auto itl = name.find("<");

				if (itl != std::string::npos && itl < it) {
					break;
				}

				name = name.substr(it + 2);
			}

			return name;
		}
	}();

	return name;
}

template <class T>
const std::string& get_type_name(const T&) {
	return get_type_name<T>();
}

template <class T>
const std::string& get_type_name_strip_namespace(const T&) {
	return get_type_name_strip_namespace<remove_cref<T>>();
}

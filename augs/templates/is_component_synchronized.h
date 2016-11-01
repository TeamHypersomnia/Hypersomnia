#pragma once
#include <type_traits>

struct synchronizable_component {

};

template<typename T>
struct is_component_synchronized {
	static constexpr bool value = std::is_base_of<synchronizable_component, T>::value;
};

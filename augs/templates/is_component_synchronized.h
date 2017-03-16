#pragma once
#include <type_traits>

struct synchronizable_component {

};

template<typename T>
struct is_component_synchronized 
	: std::bool_constant<std::is_base_of_v<synchronizable_component, T>>
{
};

#pragma once
#include <type_traits>

struct synchronizable_component {

};

template <typename T>
constexpr bool is_component_synchronized = std::is_base_of_v<synchronizable_component, T>;
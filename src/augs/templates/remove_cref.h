#pragma once
#include <type_traits>

/* 
	Normally, someone'd use std::decay_t but it uses more templates that we need.
	All the time, we actually just need to remove the const and the reference.
*/

template <class T>
using remove_cref = std::remove_const_t<std::remove_reference_t<T>>;
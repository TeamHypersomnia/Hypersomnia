#pragma once
#include <type_traits>
#include "augs/templates/remove_cref.h"

template<bool is_const, class T>
using maybe_const_ref_t = std::conditional_t<is_const, const T&, T&>;

template<bool is_const, class T>
using maybe_const_ptr_t = std::conditional_t<is_const, const T*, T*>;

template <class T>
struct is_const_ref 
	: std::bool_constant<std::is_const_v<std::remove_reference_t<T>>>
{
};

template <class T>
struct is_const_ptr 
	: std::bool_constant<std::is_const_v<std::remove_pointer_t<T>>>
{
};

template <class T>
constexpr bool is_const_ptr_v = is_const_ptr<T>::value;

template <class T>
constexpr bool is_const_ref_v = is_const_ref<T>::value;

template <class>
struct is_handle_const;

template <
	template <bool, class, template<class> class> class H, 
	bool A, 
	class B, 
	template <class> class C
>
struct is_handle_const <H<A, B, C>> : std::bool_constant<A> {};

template <
	template <bool> class H, 
	bool A
>
struct is_handle_const <H<A>> : std::bool_constant<A> {};

template <class T>
constexpr bool is_handle_const_v = is_handle_const<T>::value;
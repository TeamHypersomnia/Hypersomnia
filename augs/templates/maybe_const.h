#pragma once

namespace templates_detail {
	template<bool _Test,
		class _Ty1,
		class _Ty2>
		struct conditional
	{	// type is _Ty2 for assumed !_Test
		typedef _Ty2 type;
	};

	template<class _Ty1,
		class _Ty2>
		struct conditional<true, _Ty1, _Ty2>
	{	// type is _Ty1 for _Test
		typedef _Ty1 type;
	};
}

template<bool is_const, class T>
struct maybe_const_ref { typedef typename templates_detail::conditional<is_const, const T&, T&>::type type; };

template<bool is_const, class T>
using maybe_const_ref_t = typename maybe_const_ref<is_const, T>::type;

template<bool is_const, class T>
struct maybe_const_ptr { typedef typename templates_detail::conditional<is_const, const T*, T*>::type type; };

template<bool is_const, class T>
using maybe_const_ptr_t = typename maybe_const_ptr<is_const, T>::type;

template <class T>
struct is_const_ref {
	static constexpr bool value = std::is_const_v<std::remove_reference_t<T>>;
};
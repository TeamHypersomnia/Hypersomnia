// This has been stripped from boost minus the compatibility for borland etc.
//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/utility for most recent version including documentation.

// call_traits: defines typedefs for function usage
// (see libs/utility/call_traits.htm)

/* Release notes:
23rd July 2000:
Fixed array specialization. (JM)
Added Borland specific fixes for reference types
(issue raised by Steve Cleary).
*/

#ifndef LUABIND_CALL_TRAITS_HPP_INCLUDED
#define LUABIND_CALL_TRAITS_HPP_INCLUDED

namespace luabind {
	namespace detail {

		template <typename T, bool small_>
		struct ct_imp2
		{
			typedef const T& param_type;
		};

		template <typename T>
		struct ct_imp2<T, true>
		{
			typedef const T param_type;
		};

		template <typename T, bool isp, bool b1, bool b2>
		struct ct_imp
		{
			typedef const T& param_type;
		};

		template <typename T, bool isp, bool b2>
		struct ct_imp<T, isp, true, b2>
		{
			typedef typename ct_imp2<T, sizeof(T) <= sizeof(void*)>::param_type param_type;
		};

		template <typename T, bool isp, bool b1>
		struct ct_imp<T, isp, b1, true>
		{
			typedef typename ct_imp2<T, sizeof(T) <= sizeof(void*)>::param_type param_type;
		};

		template <typename T, bool b1, bool b2>
		struct ct_imp<T, true, b1, b2>
		{
			typedef const T param_type;
		};

		template <typename T>
		struct call_traits
		{
		public:
			typedef T value_type;
			typedef T& reference;
			typedef const T& const_reference;

			typedef typename ct_imp<
				T,
				std::is_pointer<T>::value,
				std::is_integral<T>::value || std::is_floating_point<T>::value,
				std::is_enum<T>::value
			>::param_type param_type;
		};

		template <typename T>
		struct call_traits<T&>
		{
			typedef T& value_type;
			typedef T& reference;
			typedef const T& const_reference;
			typedef T& param_type;
		};

		template <typename T, std::size_t N>
		struct call_traits<T[N]>
		{
		private:
			typedef T array_type[N];
		public:
			typedef const T* value_type;
			typedef array_type& reference;
			typedef const array_type& const_reference;
			typedef const T* const param_type;
		};

		template <typename T, std::size_t N>
		struct call_traits<const T[N]>
		{
		private:
			typedef const T array_type[N];
		public:
			typedef const T* value_type;
			typedef array_type& reference;
			typedef const array_type& const_reference;
			typedef const T* const param_type;
		};
	}
}

#endif


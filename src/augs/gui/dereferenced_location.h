#pragma once
#include "augs/templates/maybe_const.h"

template <bool is_const, class location_type>
class basic_dereferenced_location {
	typedef typename location_type::dereferenced_type dereferenced_type;
	typedef maybe_const_ref_t<is_const, dereferenced_type> dereferenced_ref;
	typedef maybe_const_ptr_t<is_const, dereferenced_type> dereferenced_ptr;

	dereferenced_ptr ptr = nullptr;
	location_type location;

	template <bool, class>
	friend class basic_dereferenced_location;
public:

	basic_dereferenced_location(dereferenced_ptr const p = nullptr, const location_type& loc = location_type()) : ptr(p), location(loc) {}

	dereferenced_ptr operator->() const {
		return ptr;
	}

	dereferenced_ref operator*() const {
		return *ptr;
	}

	explicit operator bool() const {
		return ptr != nullptr;
	}

	bool operator==(const basic_dereferenced_location b) const {
		return ptr == b.ptr;
	}

	bool operator!=(const basic_dereferenced_location b) const {
		return !operator==(b);
	}

	bool operator==(const location_type b) const {
		return location == b;
	}

	bool operator!=(const location_type b) const {
		return !operator==(b);
	}

	operator location_type() const {
		return location;
	}

	const location_type& get_location() const {
		return location;
	}

	operator basic_dereferenced_location<true, location_type>() const {
		return{ ptr, location };
	}
};

template <class T>
using dereferenced_location = basic_dereferenced_location<false, T>;

template <class T>
using const_dereferenced_location = basic_dereferenced_location<true, T>;

template<class T>
auto make_dereferenced_location(typename T::dereferenced_type* p, const T& l) {
	return dereferenced_location<T>(p, l);
}

template<class T>
auto make_dereferenced_location(const typename T::dereferenced_type* p, const T& l) {
	return const_dereferenced_location<T>(p, l);
}
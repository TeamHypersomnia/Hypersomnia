#pragma once

template <class T>
class dereferenced_location {
	typedef typename T::location location_type;

	T* ptr = nullptr;
	location_type location;

	template <class>
	friend class dereferenced_location;
public:

	dereferenced_location(T* const p = nullptr, const location_type& loc = location_type()) : ptr(p), location(loc) {}

	T* operator->() const {
		return ptr;
	}

	T& operator*() const {
		return *ptr;
	}

	operator bool() const {
		return ptr != nullptr;
	}

	bool operator==(const dereferenced_location<const T> b) const {
		return ptr == b.ptr;
	}

	bool operator!=(const dereferenced_location<const T> b) const {
		return !operator==(b);
	}

	operator location_type() const {
		return location;
	}

	location_type& get_location() {
		return location;
	}

	const location_type& get_location() const {
		return location;
	}

	operator dereferenced_location<const T>() const {
		return{ ptr, location };
	}
};

template<class T>
dereferenced_location<T> make_location_and_pointer(T* p, const typename T::location& l) {
	return dereferenced_location<T>(p, l);
}

template<class T>
dereferenced_location<const T> make_location_and_pointer(const T* p, const typename T::location& l) {
	return dereferenced_location<const T>(p, l);
}
#pragma once

template <class T>
class location_and_pointer {
	typedef typename T::location location_type;

	T* ptr = nullptr;
	location_type location;
public:

	location_and_pointer(T* p = nullptr, const location_type& loc = location_type()) : ptr(p), location(loc) {}

	T* operator->() const {
		return ptr;
	}

	T& operator*() const {
		return *ptr;
	}

	operator bool() const {
		return ptr != nullptr;
	}

	bool operator==(T* b) const {
		return ptr == b;
	}

	bool operator!=(T* b) const {
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
};

template<class T>
location_and_pointer<T> make_location_and_pointer(T* p, const typename T::location& l) {
	return location_and_pointer<T>(p, l);
}

template<class T>
location_and_pointer<const T> make_location_and_pointer(const T* p, const typename T::location& l) {
	return location_and_pointer<const T>(p, l);
}
#pragma once

template <class tuple_type>
class tuple_of_containers {
protected:
	tuple_type all;

public:
	template <class T>
	auto& get_store_by(const T = T()) {
		return get_container_with_key_type<T>(all);
	}

	template <class T>
	const auto& get_store_by(const T = T()) const {
		return get_container_with_key_type<T>(all);
	}

	template <class T>
	decltype(auto) find(const T id) {
		return found_or_nullptr(get_store_by(id), id);
	}

	template <class T>
	decltype(auto) find(const T id) const {
		return found_or_nullptr(get_store_by(id), id);
	}

	template <class T>
	decltype(auto) operator[](const T id) {
		return get_store_by(id)[id];
	}

	template <class T>
	decltype(auto) at(const T id) {
		return get_store_by(id).at(id);
	}

	template <class T>
	decltype(auto) at(const T id) const {
		return get_store_by(id).at(id);
	}
};
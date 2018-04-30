#pragma once
#include "augs/templates/folded_finders.h"
#include "augs/templates/filter_types.h"

template <class... Args>
struct special_widgets {
	std::tuple<Args...> handlers;

	template <class T>
	static constexpr bool handles = (... || Args::template handles<T>);

	special_widgets(Args&&... args) : handlers(std::move(args)...) {}

	template <class T>
	struct detail_handles {
		template <class C>
		struct type : std::bool_constant<C::template handles<T>> {};
	};

	template <class T>
	decltype(auto) describe_changed(
		const std::string& formatted_label,
		const T& to
	) const {
		using matching = find_matching_type_in_list<detail_handles<T>::template type, type_list<Args...>>;
		return std::get<matching>(handlers).describe_changed(formatted_label, to);
	}

	template <class T>
	decltype(auto) handle(
		const std::string& identity_label, 
		T& object
	) const {
		using matching = find_matching_type_in_list<detail_handles<T>::template type, type_list<Args...>>;
		return std::get<matching>(handlers).handle(identity_label, object);
	}
};


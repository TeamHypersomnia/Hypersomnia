#pragma once
#include "augs/templates/get_by_dynamic_id.h"
#include "augs/templates/get_type_name.h"

template <class T>
std::string get_current_type_name(const type_in_list_id<T>& id) {
	return get_by_dynamic_id(
		typename type_in_list_id<T>::list_type(),
		id,
		[](auto e){
			return get_type_name(e);
		}
	);
}
#pragma once
#include "augs/templates/type_matching_and_indexing.h"

template <class Archive>
struct byte_type_for {
	using type = std::conditional_t<
		is_derived_from_any_of_v<
			Archive, 
			std::ofstream, 
			std::ifstream
		>, 
		char, 
		std::byte
	>;
};

template <class Archive>
using byte_type_for_t = typename byte_type_for<Archive>::type;

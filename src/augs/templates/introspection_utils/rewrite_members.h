#pragma once
#include "augs/templates/introspect.h"

template <
	class destination_type,
	template <class> class T,
	class source_type,
	class F
>
T<destination_type> rewrite_members_and_transform_templated_type_into(
	const T<source_type>& source,
	F transformator,
	std::enable_if_t<!std::is_same_v<destination_type, source_type>>* = nullptr
) {
	T<destination_type> destination;

	auto rewrite_rest_of_the_members =
		[](
			auto, 
			auto& rewritten_to, 
			const auto& rewritten_from
		) {
			rewritten_to = rewritten_from;
		}
	;

	typedef concat_unary_t<
		std::conjunction,
		bind_types_t<std::is_base_of, destination_type>,
		bind_types_t<std::is_base_of, const source_type>
	> are_of_transformed_types;

	augs::introspect_recursive<
		apply_to_arguments_t<std::is_assignable, std::add_lvalue_reference_t>,
		apply_negation_t<are_of_transformed_types>,
		stop_recursion_if_valid
	>(
		rewrite_rest_of_the_members,
		destination,
		source
	);

	augs::introspect_recursive<
		are_of_transformed_types,
		always_recurse,
		stop_recursion_if_valid
	>(
		[transformator](
			auto, 
			auto& destination_member, 
			const auto& source_member
		) {
			transformator(destination_member, source_member);
		},
		destination,
		source
	);

	return destination;
}
template <
	class destination_type,
	template <class> class T,
	class F
>
T<destination_type> rewrite_members_and_transform_templated_type_into(
	const T<destination_type>& source,
	F transformator
) {
	return source;
}
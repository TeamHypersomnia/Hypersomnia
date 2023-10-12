#define INCLUDE_TYPES_IN 1
#include <fstream>
#include "augs/filesystem/path.h"
#include "augs/templates/type_templates.h"
#include "augs/templates/maybe.h"

#include "augs/templates/traits/is_comparable.h"
#include "augs/templates/type_map.h"
#include "augs/templates/logically_empty.h"
#include "augs/readwrite/custom_lua_representations.h"
#include "augs/readwrite/custom_json_representations.h"

#include "augs/readwrite/memory_stream.h"

#include "game/cosmos/cosmos.h"
#include "game/organization/all_component_includes.h"

#include "augs/templates/get_index_type_for_size_of.h"

#include "game/assets/all_logical_assets.h"

#include "augs/templates/traits/container_traits.h"
#include "augs/templates/traits/is_enum_map.h"
#include "game/components/pathfinding_component.h"
#include "game/organization/for_each_entity_type.h"
#include "game/organization/for_each_component_type.h"
#include "game/cosmos/entity_handle.h"
#include "game/modes/mode_entropy.h"

#include "augs/pad_bytes.h"

#include "augs/templates/introspection_utils/types_in.h"
#include "augs/templates/filter_types.h"

#include "augs/readwrite/lua_readwrite.h"
#include "augs/readwrite/byte_readwrite.h"
#include "view/viewables/all_viewables_defs.h"

#include "augs/misc/maybe_official_path.h"

#include "3rdparty/imgui/imgui.h"

#include "augs/misc/constant_size_string.h"
/* Define several other traits which will validate properties of some other types. */

namespace templates_detail {
	template <class T>
	struct identity {
		using type = T;
	};

	template <class T>
	using identity_t = typename identity<T>::type;
}

/* Define several other types which will be needed as input for some tested traits. */

struct derivedintrotest : basic_ltrb<float> {
	using introspect_base = basic_ltrb<float>;
};

struct derivedstreamtest : augs::cref_memory_stream {};

using test_type_map = type_map<
	type_pair<int, double>,
	type_pair<double, int>,
	type_pair<float, const char*>,
	type_pair<const int, std::string>
>;

using test_type_value_map = type_value_map<uint32_t,
	static_cast<uint32_t>(-1),
	type_uint32_pair<int, 2>,
	type_uint32_pair<double, 10>,
	type_uint32_pair<float, 5931>,
	type_uint32_pair<const int, 0>
>;

void ftrait_test(int, double, char);

struct ftrait {
	void test(int, double, char);
	void ctest(int, double, char) const;
};

/* A shortcut which will be heavily used from now on */

template <class A, class B>
constexpr bool same = std::is_same_v<A, B>;

struct tests_of_traits {
	/* Functional tests. */

	static void test_get_by_dynamic_id() {
		all_entity_types t;

		auto okay = get_by_dynamic_index(t, std::size_t(0), [](auto){
			return 20.0;	
		});

		auto okay2 = get_by_dynamic_id(t, type_in_list_id<all_entity_types>(), [](auto){
			return 20.0;	
		});

		using candidates = type_list<plain_missile>;

		auto tester = [](auto a) -> decltype(auto) {
			using T = remove_cref<decltype(a)>;
			static_assert(same<T, plain_missile>);
			return 20.0;	
		};

		auto nopt_tester = [](const auto& a) -> decltype(auto) {
			using T = remove_cref<decltype(a)>;

			if constexpr(same<T, std::nullopt_t>) {
				return 10.0;
			}
			else {
				return double(num_types_in_list_v<typename T::component_list>);
			}
		};

		auto okay3 = constrained_get_by_dynamic_index<candidates>(t, std::size_t(0), tester);
		auto okay4 = constrained_get_by_dynamic_id<candidates>(t, type_in_list_id<all_entity_types>(), tester);

		static_assert(same<double, decltype(okay)>);
		static_assert(same<double, decltype(okay2)>);
		static_assert(same<double, decltype(okay3)>);
		static_assert(same<double, decltype(okay4)>);

		{
			auto okay5 = constrained_find_by_dynamic_id<candidates>(t, type_in_list_id<all_entity_types>(), nopt_tester);
			static_assert(same<double, decltype(okay5)>);
		}

		augs::pool<int, of_size<300>::make_nontrivial_constant_vector, unsigned short> ppp;

		// ERROR: "The container can hold more elements than the pool can index with size_type!"
		// augs::pool<int, of_size<300>::make_nontrivial_constant_vector, unsigned char> ppp2;
	}

	/* One-shot asserts. */

	static_assert(!statically_allocate_entities, "Let's keep it that way for now");
	static_assert(same<int, argument_t<decltype(&ftrait::test), 0>>);
	static_assert(same<int, argument_t<decltype(&ftrait::ctest), 0>>);

	static_assert(same<double, argument_t<decltype(ftrait_test), 1>>);
	static_assert(same<char, argument_t<decltype(ftrait_test), 2>>);
	static_assert(same<char, last_argument_t<decltype(ftrait_test)>>);

	static_assert(same<char, last_argument_t<decltype(&ftrait::test)>>);
	static_assert(same<char, last_argument_t<decltype(&ftrait::ctest)>>);

	static_assert(!is_one_of_v<int, float, double>, "Trait has failed");
	static_assert(is_one_of_v<unsigned, float, float, double, unsigned>, "Trait has failed");
	static_assert(is_one_of_v<cosmos, int, cosmos_clock, cosmos>, "Trait has failed");
	static_assert(same<test_type_map::at<int>, double>);
	static_assert(same<test_type_map::at<double>, int>);
	static_assert(same<test_type_map::at<float>, const char*>);
	static_assert(same<test_type_map::at<const int>, std::string>);

	static_assert(2 == test_type_value_map::at<int>);
	static_assert(10 == test_type_value_map::at<double>);
	static_assert(5931 == test_type_value_map::at<float>);
	static_assert(0 == test_type_value_map::at<const int>);
	static_assert(static_cast<uint32_t>(-1) == test_type_value_map::at<const double>);

	static_assert(augs::is_byte_stream_v<augs::memory_stream>);
	static_assert(augs::is_byte_stream_v<augs::cref_memory_stream>);
	static_assert(augs::is_byte_stream_v<derivedstreamtest>);
	static_assert(!augs::is_byte_stream_v<derivedintrotest>);

	static_assert(augs::is_byte_stream_v<std::ifstream>);
	static_assert(augs::is_byte_stream_v<std::ofstream>);

	static_assert(has_introspect_base_v<child_entity_id>);

	static_assert(!has_introspect_base_v<int>);
	static_assert(!has_introspect_base_v<double>);
	static_assert(!has_introspect_base_v<value_meter>);

	static_assert(has_introspect_v<derivedintrotest>, "Trait has failed");
	static_assert(has_introspect_body_v<value_meter>, "Trait has failed");

	static_assert(has_member_find_v<std::unordered_map<int*, int*>, int*>);
	static_assert(!has_member_find_v<std::vector<int>, int>);

	static_assert(member_find_returns_ptr_v<augs::pool<int, make_vector, unsigned>, augs::pooled_object_id<unsigned>>);
	static_assert(!member_find_returns_ptr_v<std::unordered_map<int, int*>, int>);
	static_assert(!member_find_returns_ptr_v<std::unordered_map<int*, int*>, int*>);

	static_assert(has_string_v<augs::path_type>);
	static_assert(has_string_v<const augs::path_type&>);
	static_assert(has_string_v<augs::path_type&>);

	static_assert(!value_conjunction<true, false, true, false>::value);
	static_assert(value_disjunction<true, false, true, false>::value);

	static_assert(augs::is_pool_v<augs::pool<int, make_vector, unsigned>>);
	static_assert(augs::is_pool_v<augs::pool<int, of_size<300>::make_nontrivial_constant_vector, unsigned short>>);
	static_assert(!augs::is_pool_v<std::vector<int>>);

	static_assert(is_comparable_v<const int, int>);
	//static_assert(!is_comparable_v<typed_entity_id<controlled_character>, typed_entity_id<plain_missile>>);

	static_assert(std::is_convertible_v<typed_entity_id<controlled_character>, entity_id>);
	static_assert(!std::is_constructible_v<typed_entity_id<controlled_character>, entity_id>);
	static_assert(!std::is_convertible_v<entity_id, typed_entity_id<controlled_character>>);

	static_assert(std::is_convertible_v<typed_entity_flavour_id<controlled_character>, entity_flavour_id>);
	static_assert(!std::is_constructible_v<typed_entity_flavour_id<controlled_character>, entity_flavour_id>);
	static_assert(!std::is_convertible_v<entity_flavour_id, typed_entity_flavour_id<controlled_character>>);

	static_assert(has_any_of_v<controlled_character, invariants::sprite, invariants::polygon>);
	static_assert(!has_any_of_v<controlled_character, invariants::trace>);

	//static_assert(std::is_trivially_copyable_v<absolute_or_local>);
	static_assert(same<double, type_argument_t<std::is_trivially_copyable<double>>>);
	static_assert(same<constrained_entity_flavour_id<invariants::missile>::matching_types, type_list<plain_missile>>);

	static_assert(ref_typed_entity_handle<controlled_character>::is_typed);
	static_assert(!const_entity_handle::is_typed);

	static_assert(all_are_v<std::is_trivially_copyable, type_list<int, double, float>>);

	static_assert(same<
		type_list<int, int, double, double>,
		concatenate_lists_t<type_list<int, int>, type_list<double, double>>
	>);

	static_assert(same<
		std::tuple<int, int, double, double>,
		concatenate_lists_t<std::tuple<int, int>, type_list<double, double>>
	>);

	static_assert(is_handle_const_v<const_entity_handle>);
	static_assert(!is_handle_const_v<iterated_entity_handle<controlled_character>>);
	static_assert(is_handle_const_v<const_iterated_entity_handle<controlled_character>>);

	static_assert(!can_reserve_caches_v<flavour_id_cache>);
	static_assert(can_reserve_caches_v<physics_world_cache>);
	static_assert(std::is_trivially_copyable_v<game_intent_type>);
	static_assert(has_string_v<augs::path_type>);

	static_assert(static_cast<int>(imguicol_helper::ImGuiCol_COUNT) == static_cast<int>(ImGuiCol_COUNT));

	static_assert(augs::has_byte_readwrite_overloads_v<augs::memory_stream, augs::path_type>);
	static_assert(!augs::has_byte_readwrite_overloads_v<augs::memory_stream, cosmos>);
	static_assert(!augs::has_lua_readwrite_overloads_v<cosmos>);
	static_assert(!augs::has_lua_readwrite_overloads_v<value_meter>);

	static_assert(!augs::has_byte_readwrite_overloads_v<augs::memory_stream, value_meter>);

	static_assert(is_container_v<augs::path_type>);
	static_assert(!is_padding_field_v<entity_id>);
	static_assert(is_padding_field_v<pad_bytes<4>>);
	static_assert(is_padding_field_v<pad_bytes<1>>);

	static_assert(!has_introspect_v<cosmos>, "Trait has failed");
	static_assert(has_introspect_v<cosmos_clock>, "Trait has failed");
	//static_assert(has_introspect_v<augs::constant_size_vector<int, 2>>, "Trait has failed");
	static_assert(has_introspect_v<augs::delta>, "Trait has failed");
	static_assert(alignof(meter_instance_tuple) == 4, "Trait has failed");

	static_assert(sum_sizes_until_nth_v<0, std::tuple<int, double, float>> == 0, "Trait has failed");
	static_assert(sum_sizes_until_nth_v<1, std::tuple<int, double, float>> == 4, "Trait has failed");
	static_assert(sum_sizes_until_nth_v<2, std::tuple<int, double, float>> == 12, "Trait has failed");
	static_assert(sum_sizes_until_nth_v<3, std::tuple<int, double, float>> == 16, "Trait has failed");

	static_assert(sum_sizes_of_types_in_list_v<std::tuple<int, double, float>> == 16, "Trait has failed");
	static_assert(sum_sizes_of_types_in_list_v<std::tuple<int>> == 4, "Trait has failed");
	static_assert(sum_sizes_of_types_in_list_v<std::tuple<>> == 0, "Trait has failed");

	static_assert(!has_constexpr_size_v<std::vector<int>>, "Trait has failed");
	static_assert(has_constexpr_size_v<std::array<int, 3>>, "Trait has failed");
	static_assert(has_constexpr_max_size_v<augs::enum_array<int, item_holding_stance>>, "Trait has failed");
	static_assert(has_constexpr_max_size_v<std::array<int, 3>>, "Trait has failed");
	static_assert(has_constexpr_max_size_v<augs::constant_size_vector<int, 20>>, "Trait has failed");
	static_assert(has_constexpr_size_v<std::array<vec2, 3>>, "Trait has failed");
	static_assert(has_constexpr_size_v<decltype(pad_bytes<3>::pad)>, "Trait has failed");

	static_assert(!has_constexpr_max_size_v<std::vector<int>>, "Trait has failed");

	static_assert(is_std_array_v<decltype(pad_bytes<3>::pad)>, "Trait has failed");
	static_assert(is_container_v<augs::enum_map<game_intent_type, vec2>>, "Trait has failed");
	static_assert(is_container_v<augs::enum_array<basic_transform<float>, colliders_offset_type>>, "Trait has failed");
	static_assert(is_container_v<augs::enum_array<basic_transform<float>, colliders_offset_type>>, "Trait has failed");
	static_assert(!is_container_v<int>, "Trait has failed");
	static_assert(is_container_v<std::string>, "Trait has failed");
	static_assert(is_enum_array_v<augs::enum_array<std::string, item_holding_stance>>, "Trait has failed");

	static_assert(is_container_v<augs::constant_size_vector<vec2, 20>>, "Trait has failed");
	static_assert(augs::is_byte_readwrite_appropriate_v<augs::memory_stream, augs::constant_size_vector<vec2, 20>>, "Trait has failed");
	static_assert(augs::is_byte_readwrite_appropriate_v<augs::memory_stream, augs::enum_map<game_intent_type, vec2>>, "Trait has failed");
	static_assert(augs::is_byte_readwrite_appropriate_v<augs::memory_stream, augs::enum_boolset<game_intent_type>>, "Trait has failed");
	static_assert(is_container_v<std::vector<int>>, "Trait has failed");
	static_assert(is_container_v<std::vector<vec2>>, "Trait has failed");
	static_assert(is_container_v<std::vector<cosmos>>, "Trait has failed");
	static_assert(is_container_v<std::vector<pathfinding_session>>, "Trait has failed");

	static_assert(can_access_data_v<std::string>, "Trait has failed");
	static_assert(can_access_data_v<std::vector<int>>, "Trait has failed");
	static_assert(!can_access_data_v<std::map<int, int>>, "Trait has failed");
	static_assert(can_reserve_v<std::vector<int>>, "Trait has failed");
	static_assert(!can_reserve_v<std::map<int, int>>, "Trait has failed");

	static_assert(!has_introspect_v<unsigned>, "Trait has failed");
	static_assert(has_introspect_v<basic_ltrb<float>>, "Trait has failed");
	static_assert(has_introspect_v<basic_ltrb<int>>, "Trait has failed");

	static_assert(same<filter_types_in_list<std::is_integral, type_list<double, int, float>>::types, type_list<int>>, "Trait has failed");
	static_assert(same<filter_types_in_list<std::is_integral, type_list<double, int, float>>::get_type<0>, int>, "Trait has failed");
	
	static_assert(is_one_of_list_v<unsigned, std::tuple<float, float, double, unsigned>>, "Trait has failed");

	static_assert(index_in_list_v<unsigned, std::tuple<float, float, double, unsigned>> == 3, "Trait has failed");
	static_assert(index_in_v<unsigned, float, float, double, unsigned> == 3, "Trait has failed");
	
	static_assert(same<unsigned, nth_type_in_t<0, unsigned, float, float>>, "Trait has failed");
	static_assert(same<float, nth_type_in_t<1, unsigned, float, float>>, "Trait has failed");
	static_assert(same<float, nth_type_in_t<2, unsigned, float, float>>, "Trait has failed");
	static_assert(same<double, nth_type_in_t<3, unsigned, float, float, double, unsigned>>, "Trait has failed");


	static_assert(should_reinfer_when_tweaking_v<invariants::fixtures>);
	static_assert(should_reinfer_when_tweaking_v<invariants::rigid_body>);

	static_assert(!should_reinfer_when_tweaking_v<invariants::trace>);
	
	static_assert(
		same<
			filter_types_in_list<std::is_integral, type_list<int, double, float, unsigned>>::types, 
			type_list<int, unsigned>
		>, 
		"Trait has failed"
	);

	static_assert(
		!same<
		filter_types_in_list<std::is_floating_point, type_list<int, double, float, unsigned>>::types,
			std::tuple<int, unsigned>
		>, 
		"Trait has failed"
	);

	static_assert(
		same<type_list<int&, double&, float&>, transform_types_in_list_t<type_list<int, double, float>, std::add_lvalue_reference_t>>,
		"Trait has failed."
	);
	
	static_assert(
		same<type_list<int, double, float>, transform_types_in_list_t<type_list<const int&, double&&, float&>, remove_cref>>,
		"Trait has failed."
	);
	
	static_assert(
		same<type_list<const int&, double&&, float&>, transform_types_in_list_t<type_list<const int&, double&&, float&>, templates_detail::identity_t>>,
		"Trait has failed."
	);

	static_assert(same<unsigned char, get_index_type_for_size_of_t<char>>, "Trait has failed");
	static_assert(same<unsigned char, get_index_type_for_size_of_t<char[255]>>, "Trait has failed");
	static_assert(same<unsigned char, get_index_type_for_size_of_t<char[256]>>, "Trait has failed");
	static_assert(same<unsigned short, get_index_type_for_size_of_t<char[257]>>, "Trait has failed");
	static_assert(same<unsigned short, get_index_type_for_size_of_t<char[65536]>>, "Trait has failed");
	static_assert(same<unsigned int, get_index_type_for_size_of_t<char[65537]>>, "Trait has failed");

	//static_assert(sizeof(cosmos) < 1000000, "Possible stack overflow due to cosmos on the stack");


	static_assert(is_introspective_leaf_v<activity_type>);

	static_assert(augs::has_custom_to_lua_value_v<augs::path_type>);
	static_assert(augs::has_custom_to_lua_value_v<rgba>);
	static_assert(augs::has_custom_to_lua_value_v<ImVec4>);
	static_assert(!augs::has_custom_to_lua_value_v<int>);
	static_assert(!augs::has_custom_to_lua_value_v<std::map<int, int>>);

	static_assert(aligned_num_of_bytes_v<0, 4> == 0, "Trait is wrong");
	static_assert(aligned_num_of_bytes_v<1, 4> == 4, "Trait is wrong");
	static_assert(aligned_num_of_bytes_v<2, 4> == 4, "Trait is wrong");
	static_assert(aligned_num_of_bytes_v<3, 4> == 4, "Trait is wrong");
	static_assert(aligned_num_of_bytes_v<4, 4> == 4, "Trait is wrong");
	static_assert(aligned_num_of_bytes_v<5, 4> == 8, "Trait is wrong");
	static_assert(aligned_num_of_bytes_v<6, 4> == 8, "Trait is wrong");
	static_assert(aligned_num_of_bytes_v<7, 4> == 8, "Trait is wrong");
	static_assert(aligned_num_of_bytes_v<8, 4> == 8, "Trait is wrong");
	static_assert(aligned_num_of_bytes_v<9, 4> == 12, "Trait is wrong");

	static_assert(same<reverse_sequence_t<std::index_sequence<1, 2, 3>>, std::index_sequence<3, 2, 1>>);

	static_assert(has_suitable_member_assign_v<std::vector<int>, std::unordered_set<double>>);
	static_assert(has_suitable_member_assign_v<std::vector<double>, std::unordered_set<int>>);
	static_assert(!has_suitable_member_assign_v<std::unordered_set<int>, std::vector<double>>);

	/* Type containment - tests of traits */

	static_assert(can_type_contain_v<std::vector<int>, int>);
	static_assert(can_type_contain_v<vec2, float>);

	static_assert(has_types_in_v<std::tuple<int, double>>);
	static_assert(has_types_in_v<augs::trivially_copyable_tuple<int, double>>);

	static_assert(has_types_in_v<vec2>);
	static_assert(has_types_in_v<augs::simple_pair<int, double>>);
	static_assert(has_types_in_v<std::pair<int, double>>);
	static_assert(!has_types_in_v<int>);

	static_assert(can_type_contain_v<std::pair<int, double>, int>);
	static_assert(!can_type_contain_v<std::pair<int, float>, double>);
	static_assert(!can_type_contain_v<std::pair<int, char>, double>);

	static_assert(can_type_contain_v<augs::simple_pair<int, double>, int>);
	static_assert(!can_type_contain_v<augs::simple_pair<int, float>, double>);
	static_assert(!can_type_contain_v<augs::simple_pair<int, char>, double>);

	static_assert(!can_type_contain_v<vec2, int>);
	static_assert(can_type_contain_v<std::vector<std::vector<double>>, double>);
	static_assert(can_type_contain_v<std::vector<std::vector<double>>, std::vector<double>>);

	static_assert(can_type_contain_v<std::optional<std::vector<double>>, std::vector<double>>);

	static_assert(can_type_contain_v<
		std::map<int, std::vector<std::unordered_map<double, char>>>, 
		char
	>);

	static_assert(can_type_contain_v<
		std::map<int, std::vector<std::unordered_map<double, char>>>, 
		std::unordered_map<double, char>
	>);

	static_assert(can_type_contain_v<
		std::map<int, std::vector<std::unordered_map<double, std::optional<std::string>>>>, 
		std::string
	>);

	static_assert(can_type_contain_v<
		std::map<int, std::vector<std::unordered_map<double, std::optional<std::string>>>>, 
		std::optional<std::string>
	>);

	static_assert(can_type_contain_v<
		std::map<int, std::vector<std::unordered_map<double, char>>>, 
		int
	>);

	static_assert(std::is_trivially_copyable_v<augs::maybe<int>>);

	static_assert(is_bound_v<augs::bound<float>>);
	static_assert(is_bound_v<augs::bound<int>>);
	static_assert(!is_bound_v<augs::simple_pair<float, int>>);

	static_assert(is_arithmetic_bound_v<augs::bound<float>>);
	static_assert(is_arithmetic_bound_v<augs::bound<int>>);

	static_assert(!is_arithmetic_bound_v<augs::bound<std::string>>);
	static_assert(!is_arithmetic_bound_v<augs::bound<augs::bound<float>>>);

	static_assert(!is_typed_flavour_id_v<assets::image_id>);
	static_assert(is_enum_map_v<decltype(invariants::container().slots)>);
	static_assert(!is_enum_map_v<int>);
};

/* 
	These checks will actually put the traits to use, in order to enforce some assumptions made in the code,
	or to check for errors in their implementation.

	For example, it will validate that the coder hasn't inserted 
	some malicious member fields into structs important for the game to function.
*/

struct game_state_checks {
	void validate_entity_types() {
		for_each_through_std_get(
			all_entity_types(), 
			[](auto e) {
				using E = decltype(e);

				for_each_through_std_get(
					assert_always_together(),
					[](auto constraint) {
						using C = decltype(constraint);
						using F = typename C::First;
						using S = typename C::Second;
						using List = invariants_and_components_of<E>;

						static_assert(
							is_one_of_list_v<F, List> == is_one_of_list_v<S, List>,
							"An entity type lacks a component/invariant to function properly."
						);
					}
				);

				for_each_through_std_get(
					assert_first_implies_second(),
					[](auto constraint) {
						using C = decltype(constraint);
						using F = typename C::First;
						using S = typename C::Second;
						using List = invariants_and_components_of<E>;

						if constexpr(is_one_of_list_v<F, List>) {
							static_assert(
								is_one_of_list_v<S, List>,
								"An entity type lacks a component/invariant to function properly."
							);
						}
					}
				);

				for_each_through_std_get(
					assert_never_together(),
					[](auto constraint) {
						using C = decltype(constraint);
						using F = typename C::First;
						using S = typename C::Second;
						using List = invariants_and_components_of<E>;

						static_assert(
							!is_one_of_list_v<F, List> || !is_one_of_list_v<S, List>,
							"An entity type defines a redundant component/invariant."
						);
					}
				);
			}
		);
	}

	struct blabla {};

	/* This will also fail if pointers or references are present */

	static_assert(!can_type_contain_v<cosmos_solvable_significant, blabla>);
	static_assert(!can_type_contain_v<cosmos_common_significant, blabla>);

	/* Invariants should not hold any ids because ids are subject to invalidation */

	void validate_no_ids_in_flavours() {
		for_each_entity_type([](auto e) {
			using E = decltype(e);
			using F = entity_flavour<E>;

			static_assert(!can_type_contain_v<decltype(F::invariant_state), entity_id_base>);
		});
	}

	template <class T>
	struct is_asset_id {
		static constexpr bool value = is_pathed_asset<T> || is_unpathed_asset<T>;
	};

	/* Components should not hold any flavour/asset ids because we could not afford to look at props of every entity, for now */

	/*
		Known exceptions:
		damage_cause::flavour
		damage_sender::direct_sender_flavour
	*/

	//static_assert(2 == sum_matching_in_v<is_flavour_id, cosmos_solvable_significant>);

	/* A temporary exception for the portal component */
	static_assert(sum_matching_in_v<is_asset_id, cosmos_solvable_significant> == 6);

	/* 
		Assets should not hold any flavour id, because flavours are conceptually higher-level than assets. 
		Flavours, on the other hand, can obviously hold other flavour ids and other asset ids.

		Notice that assets can hold other assets, e.g. animation can have image ids,
		or particle effects can have even animations.
	*/

	static_assert(!sum_matching_in_v<is_flavour_id, all_logical_assets>);
	static_assert(!sum_matching_in_v<is_flavour_id, all_viewables_defs>);

	/* Other sanity checks. */

	static_assert(!can_type_contain_v<all_logical_assets, entity_id_base>);
	static_assert(!can_type_contain_v<all_viewables_defs, entity_id_base>);

	static_assert(can_type_contain_constructible_from_v<invariants::cartridge, typed_entity_flavour_id<shootable_charge>>);
	static_assert(can_type_contain_constructible_from_v<cosmos_common_significant, typed_entity_flavour_id<shootable_charge>>);
	static_assert(!can_type_contain_constructible_from_v<invariants::cartridge, typed_entity_flavour_id<static_light>>);

	static_assert(!can_type_contain_constructible_from_v<invariants::fixtures, typed_entity_flavour_id<plain_missile>>);
	static_assert(!can_type_contain_constructible_from_v<invariants::flags, typed_entity_flavour_id<plain_missile>>);

	using S = int;
	using some_pool_type = augs::pool<S, of_size<200>::template make_nontrivial_constant_vector, cosmic_pool_size_type>;

	/* Ensures that the byte-wise writer is not invoked for a pool */
	static_assert(!std::is_trivially_copyable_v<some_pool_type>);

	static_assert(is_unique_ptr_v<std::unique_ptr<int>>);
	static_assert(!is_unique_ptr_v<std::optional<int>>);
	static_assert(is_comparable_v<augs::maybe<float>, augs::maybe<float>>);
	static_assert(is_comparable_v<const augs::maybe<float>&, const augs::maybe<float>&>);

	static_assert(is_container_v<augs::constant_size_string<20>>);

	static_assert(has_empty_v<mode_entropy>);
	static_assert(has_empty_v<std::vector<int>>);
	static_assert(!has_empty_v<int>);
};

#include "generated/enums.cpp"

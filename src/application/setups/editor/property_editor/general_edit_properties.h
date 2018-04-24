#pragma once
#include "augs/pad_bytes.h"
#include "game/assets/ids/is_asset_id.h"
#include "augs/templates/traits/is_value_with_flag.h"
#include "augs/templates/folded_finders.h"
#include "augs/string/string_templates_declaration.h"
#include "augs/drawing/flip.h"

#include "application/setups/editor/editor_settings.h"
#include "application/setups/editor/property_editor/property_editor_structs.h"
#include "application/setups/editor/property_editor/property_editor_settings.h"
#include "application/setups/editor/property_editor/default_control_provider.h"
#include "application/setups/editor/property_editor/changes_describers.h"

#include "augs/string/format_enum.h"
#include "augs/misc/imgui/imgui_enum_combo.h"

#include "application/setups/editor/detail/maybe_different_colors.h"

template <class M>
auto get_type_id_for_field() {
	decltype(field_address::type_id) id;

	if constexpr(std::is_trivially_copyable_v<M>) {
		id.set<augs::trivial_type_marker>();
	}
	else {
		id.set<M>();
	}

	return id;
}

template <class O, class M>
auto make_field_address(const O& object, const M& member) {
	field_address result;

	result.type_id = get_type_id_for_field<M>();
	result.offset = static_cast<unsigned>(
		reinterpret_cast<const std::byte*>(std::addressof(member))
		- reinterpret_cast<const std::byte*>(std::addressof(object))
	);

	return result;
}

template <class O, class M>
auto make_field_address(const M O::* const member) {
	static const O o;
	return make_field_address(o, o.*member);
}

enum class tweaker_type {
	CONTINUOUS,
	DISCRETE
};

template <class M>
struct tweaker_input {
	const tweaker_type type;
	const std::string& field_name;
	const field_address field_location;
	const M& old_value;
	const M& new_value;
};

template <class F, class S>
struct detail_edit_properties_input {
	const F tweaker_callback;
	const S special_control_provider;
	const property_editor_settings& settings;
	const int extra_columns;
};

template <
	template <class> class SkipPredicate = always_false,
	class I,
	class T
>
void detail_general_edit_properties(
	const I input,
	const T& object
) {
	using namespace augs::imgui;
	using S = std::decay_t<decltype(input.special_control_provider)>;

	const auto& do_tweaker = input.tweaker_callback;

	augs::introspect(
		augs::recursive([&](auto self, const auto& label, const auto& original_member, const bool nodify_introspected = true) {
			using M = std::decay_t<decltype(original_member)>;

			static constexpr bool should_skip = 
				is_padding_field_v<M> 
				|| SkipPredicate<M>::value
			;

			if constexpr(!should_skip) {
				const auto formatted_label = format_field_name(label);

				auto make_input = [&formatted_label, &object](const tweaker_type type, const auto& original, const auto& altered) {
					using MM = std::remove_const_t<std::remove_reference_t<decltype(original)>>;

					return tweaker_input<MM> {
						type,
						formatted_label,
						::make_field_address(object, original),
						original,
						altered
					};
				};

				const auto identity_label = std::string("##") + label;

				const auto& original = original_member;
				auto altered = original;

				if constexpr(S::template handles<M>) {
					auto in = make_input(tweaker_type::DISCRETE, original, altered);

					do_tweaker(in, [&]() { 
						return input.special_control_provider.handle(
							identity_label, 
							altered,
							in.field_location
						);
					});
				}
				else if constexpr(std::is_same_v<M, b2Filter>) {
					// TODO: checkbox matrix
					return;
				}
				else if constexpr(std::is_same_v<M, std::string>) {
					auto in = make_input(tweaker_type::CONTINUOUS, original, altered);

					do_tweaker(in, [&]() { 
						if (label == "description") {
							return input_multiline_text<512>(identity_label, altered, 8);
						}
						else if (label == "name") {
							return input_text<256>(identity_label, altered);
						}

						return input_text<256>(identity_label, altered);
					});

					/* next_column_text(); */
				}
				else if constexpr(std::is_same_v<M, bool>) {
					auto in = make_input(tweaker_type::DISCRETE, original, altered);

					do_tweaker(in, [&]() { 
						return checkbox(identity_label, altered);
					});
				}
				else if constexpr(std::is_arithmetic_v<M>) {
					auto in = make_input(tweaker_type::CONTINUOUS, original, altered);

					do_tweaker(in, [&]() { 
						return drag(identity_label, altered); 
					});

					/* next_column_text(get_type_name<M>()); */
				}
				else if constexpr(is_one_of_v<M, vec2, vec2i>) {
					auto in = make_input(tweaker_type::CONTINUOUS, original, altered);

					do_tweaker(in, [&]() { 
						return drag_vec2(identity_label, altered);
					});

					/* next_column_text(); */
				}
				else if constexpr(is_minmax_v<M>) {
					auto in = make_input(tweaker_type::CONTINUOUS, original, altered);

					do_tweaker(in, [&]() { 
						return drag_minmax(identity_label, altered); 
					});

					/* next_column_text(get_type_name<typename M::first_type>() + " range"); */
				}
				else if constexpr(is_asset_id_v<M>) {

				}
				else if constexpr(std::is_enum_v<M>) {
					auto in = make_input(tweaker_type::DISCRETE, original, altered);

					do_tweaker(in, [&]() { 
						return enum_combo(identity_label, altered);
					});

					/* next_column_text(); */
				}
				else if constexpr(std::is_same_v<M, rgba>) {
					auto in = make_input(tweaker_type::CONTINUOUS, original, altered);

					do_tweaker(in, [&]() { 
						return color_edit(identity_label, altered);
					});

					/* next_column_text(); */
				}
				else {
					if constexpr(is_value_with_flag_v<M>) {
						{
							auto in = make_input(
								tweaker_type::DISCRETE,
							   	original.is_enabled,
								altered.is_enabled
							);

							do_tweaker(in, [&]() {
								if (checkbox(identity_label, altered.is_enabled)) {
									return true;
								}

								return false;
							});
						}

						auto cols = maybe_disabled_cols(input.settings, !altered.is_enabled);

						augs::recursive(self)(label, original_member.value, false);
					}
					else if constexpr(is_container_v<M>) {

					}
					else {
						if (nodify_introspected) {
							const auto object_node = scoped_tree_node_ex(formatted_label);

							ImGui::NextColumn();
							ImGui::NextColumn();

							next_columns(input.extra_columns);

							if (object_node) {
								augs::introspect(augs::recursive(self), original);
							}
						}
						else {
							auto ind = scoped_indent();

							augs::introspect(augs::recursive(self), original);
						}
					}
				}
			}
		}),
		object
	);
}

template <
	template <class> class SkipPredicate = always_false,
   	class T,
   	class G,
   	class H,
	class Eq = true_returner,
	class S = default_control_provider
>
void general_edit_properties(
	const property_editor_input prop_in,
	const T& object,
	G post_new_change_impl,
	H rewrite_last_change_impl,
	Eq field_equality_predicate = {},
	const S special_control_provider = {},
	const int extra_columns = 0
) {
	using namespace augs::imgui;

	{
		auto& last_active = prop_in.state.last_active;

		if (last_active && last_active.value() != ImGui::GetActiveID()) {
		last_active.reset();
		}
	}

	auto& old_description = prop_in.state.old_description;

	auto post_new_change = [&](
		const description_pair& description,
		const auto field_id,
		const auto& new_content
	) {
		old_description = description.of_old;
		const auto new_description = old_description + description.of_new;

		post_new_change_impl(new_description, field_id, new_content);
	};

	auto rewrite_last = [&](
		const auto& description,
		const auto& new_content
	) {
		rewrite_last_change_impl(old_description + description, new_content);
	};

	auto do_tweaker = [&](
		const auto tw_in,
		auto control_callback
	) {
		const auto type = tw_in.type;
		const auto& field_name = tw_in.field_name;
		const auto& field_location = tw_in.field_location;
		const auto& old_value = tw_in.old_value;
		const auto& new_value = tw_in.new_value;

		ImGui::Bullet();
		text(field_name);
		ImGui::NextColumn();
		auto scope = scoped_item_width(-1);


		const auto colors = ::maybe_different_value_cols(
			prop_in.settings,
			!field_equality_predicate(old_value, field_location)
		);

		if (control_callback()) {
			if (type == tweaker_type::DISCRETE) {
				post_new_change(
					::describe_changed(field_name, old_value, new_value, special_control_provider),
					field_location,
					new_value
				);
			}
			else if (type == tweaker_type::CONTINUOUS) {
				const auto this_id = ImGui::GetActiveID();
				const auto description = ::describe_changed(field_name, old_value, new_value, special_control_provider);

				auto& last_active = prop_in.state.last_active;

				if (last_active != this_id) {
					post_new_change(description, field_location, new_value);
				}
				else {
					rewrite_last(description.of_new, new_value);
				}

				last_active = this_id;
			}
		}

		ImGui::NextColumn();

		next_columns(extra_columns);
	};

	detail_general_edit_properties(
		detail_edit_properties_input<decltype(do_tweaker), S> { 
			do_tweaker, 
			special_control_provider,
			prop_in.settings,
			extra_columns
		},
		object
	);
}

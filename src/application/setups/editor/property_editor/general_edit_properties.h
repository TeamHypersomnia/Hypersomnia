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
#include "application/setups/editor/property_editor/tweaker_type.h"

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

template <class T>
static constexpr bool has_direct_control_v = 
	is_one_of_v<T, std::string, bool, vec2, vec2i, rgba>
	|| is_minmax_v<T>
	|| std::is_arithmetic_v<T>
	|| std::is_enum_v<T>
;

template <class T>
static constexpr bool always_skip_in_properties =
	is_padding_field_v<T>
	// Yet unsupported:
	|| std::is_same_v<T, b2Filter>
	|| is_asset_id_v<T>
;

template <class M>
std::optional<tweaker_type> detail_direct_edit(
	const std::string& identity_label,
	M& altered
) {
	using namespace augs::imgui;

	if constexpr(std::is_same_v<M, std::string>) {
		const auto result = [&]() { 
			if (identity_label == "##description") {
				return input_multiline_text<512>(identity_label, altered, 8);
			}
			else if (identity_label == "##name") {
				return input_text<256>(identity_label, altered);
			}

			return input_text<256>(identity_label, altered);
		}();
		
		if (result) {
			return tweaker_type::CONTINUOUS;
		}
	}
	else if constexpr(std::is_same_v<M, bool>) {
		if (checkbox(identity_label, altered)) {
			return tweaker_type::DISCRETE;
		}
	}
	else if constexpr(std::is_arithmetic_v<M>) {
		if (drag(identity_label, altered)) { 
			return tweaker_type::CONTINUOUS;
		}
	}
	else if constexpr(is_one_of_v<M, vec2, vec2i>) {
		if (drag_vec2(identity_label, altered)) { 
			return tweaker_type::CONTINUOUS;
		}
	}
	else if constexpr(is_minmax_v<M>) {
		if (drag_minmax(identity_label, altered)) { 
			return tweaker_type::CONTINUOUS;
		}
	}
	else if constexpr(std::is_enum_v<M>) {
		if (enum_combo(identity_label, altered)) { 
			return tweaker_type::DISCRETE;
		}
	}
	else if constexpr(std::is_same_v<M, rgba>) {
		if (color_edit(identity_label, altered)) { 
			return tweaker_type::CONTINUOUS;
		}
	}
	else {
		static_assert(always_false_v<M>, "Unsupported control type.");
	}

	return std::nullopt;
}

template <class M>
struct tweaker_input {
	const tweaker_type type;
	const std::string& field_name;
	field_address field_location;
	const M& old_value;
	const M& new_value;
};

template <class S, class D>
struct detail_edit_properties_input {
	const S special_control_provider;
	const D sane_defaults;
	const property_editor_settings& settings;
	const int extra_columns;
};

inline auto bulleted_property_name(const std::string& name, const int extra_cols) {
	using namespace augs::imgui;

	ImGui::Bullet();
	text(name);
	ImGui::NextColumn();

	return std::make_tuple(
		augs::imgui::scoped_item_width(-1),
		augs::scope_guard([extra_cols](){ next_columns(1 + extra_cols); })
	);
}

template <class S, class T>
constexpr bool do_handler_or_direct = 
	S::template handles<T>
	|| has_direct_control_v<T>
;

template <class S, class T>
std::optional<tweaker_type> handler_or_direct(S provider, const std::string& identity_label, T& altered) {
	if constexpr(S::template handles<T>) {
		return provider.handle(identity_label, altered);
	}
	else if constexpr(has_direct_control_v<T>) {
		return detail_direct_edit(identity_label, altered);
	}
}

inline auto node_and_columns(
	const std::string& formatted_label,
	const int extra_columns
) {
	auto object_node = augs::imgui::scoped_tree_node_ex(formatted_label);
	augs::imgui::next_columns(2 + extra_columns);
	return object_node;
};

template <
	template <class> class SkipPredicate,
	class S,
	class D,
	class Eq,
	class F,
	class T
>
void detail_general_edit_properties(
	const detail_edit_properties_input<S, D> input,
	Eq equality_predicate,
	F notify_change_of,
	const std::string& label,
	T& altered,
	const bool pass_notifier_through = false,
	const bool nodify_introspected = true
) {
	static constexpr bool should_skip = 
		always_skip_in_properties<T> 
		|| SkipPredicate<T>::value
	;

	if constexpr(!should_skip) {
		const auto formatted_label = format_field_name(label);
		const auto identity_label = "##" + label;

		if constexpr(do_handler_or_direct<S, T>) {
			auto scope = bulleted_property_name(formatted_label, input.extra_columns);
			auto colors = ::maybe_different_value_cols(input.settings, !equality_predicate(altered)); 

			if (auto result = handler_or_direct(input.special_control_provider, identity_label, altered)) {
				notify_change_of(formatted_label, *result, altered);
			}
		}
		else {
			if constexpr(is_value_with_flag_v<T>) {
				const auto all_equal = equality_predicate(altered.is_enabled);

				{
					auto scope = bulleted_property_name(formatted_label, input.extra_columns);
					auto colors = ::maybe_different_value_cols(input.settings, !all_equal); 

					if (const auto result = augs::imgui::checkbox(identity_label, altered.is_enabled)) {
						notify_change_of(formatted_label, tweaker_type::DISCRETE, altered.is_enabled);
					}
				}

				auto colors = maybe_disabled_cols(input.settings, !all_equal || !altered.is_enabled);

				detail_general_edit_properties<SkipPredicate>(input, equality_predicate, notify_change_of, label, altered.value, pass_notifier_through, false);
			}
			else if constexpr(is_container_v<T>) {
				if constexpr(can_access_data_v<T>) {
					if (auto node = node_and_columns(formatted_label, input.extra_columns)) {
						for (unsigned i = 0; i < static_cast<unsigned>(altered.size()); ++i) {
							if (altered.size() < altered.max_size()) {
								const auto button_label = typesafe_sprintf("D##%x", i);

								if (ImGui::Button(button_label.c_str())) {
									auto duplicated = altered[i];
									altered.insert(altered.begin() + i + 1, std::move(duplicated));
									notify_change_of(formatted_label, tweaker_type::DISCRETE, altered);
									break;
								}

								ImGui::SameLine();
							}

							{
								const auto button_label = typesafe_sprintf("-##%x", i);

								if (ImGui::Button(button_label.c_str())) {
									altered.erase(altered.begin() + i);
									notify_change_of(formatted_label, tweaker_type::DISCRETE, altered);
									break;
								}
							}

							ImGui::SameLine();

							auto notify_whole_element = [&](const auto& l, const tweaker_type t, auto&&... args) {
								if (pass_notifier_through) {
									notify_change_of(l, t, std::forward<decltype(args)>(args)...);
								}
								else {
									notify_change_of(formatted_label, t, altered[i]);
								}
							};

							const auto are_elements_equal = equality_predicate(altered, i);

							detail_general_edit_properties<SkipPredicate>(
								input, 
								[are_elements_equal](auto&&... args) {
									return are_elements_equal;
								},
								notify_whole_element,
								typesafe_sprintf("%x", i), 
								altered[i],
								true
							);
						}

						if (altered.size() < altered.max_size()) {
							if (ImGui::Button("+")) {
								altered.emplace_back(input.sane_defaults.template construct<typename T::value_type>());
								notify_change_of(formatted_label, tweaker_type::DISCRETE, altered);
							}

							augs::imgui::next_columns(input.extra_columns + 2);
						}
					}
				}
			}
			else {
				auto further = [&](const std::string& l, auto& m) {
					detail_general_edit_properties<SkipPredicate>(input, equality_predicate, notify_change_of, l, m, pass_notifier_through);
				};

				if (nodify_introspected) {
					if (auto node = node_and_columns(formatted_label, input.extra_columns)) {
						augs::introspect(further, altered);
					}
				}
				else {
					auto ind = augs::imgui::scoped_indent();

					augs::introspect(further, altered);
				}
			}
		}
	}
}

template <
	template <class> class SkipPredicate = always_false,
   	class T,
   	class G,
   	class H,
	class Eq = true_returner,
	class S = default_control_provider,
	class D = default_sane_default_provider
>
void general_edit_properties(
	const property_editor_input prop_in,
	T parent_altered,
	G post_new_change_impl,
	H rewrite_last_change_impl,
	Eq field_equality_predicate = {},
	const S special_control_provider = {},
	const D sane_defaults = {},
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

	auto do_tweaker = [&](const auto tw_in) {
		const auto type = tw_in.type;
		const auto& field_name = tw_in.field_name;
		const auto& field_location = tw_in.field_location;
		const auto& old_value = tw_in.old_value;
		const auto& new_value = tw_in.new_value;

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
	};

	auto traverse = [&](const std::string& member_label, auto& member) {
		detail_general_edit_properties<SkipPredicate>(
			detail_edit_properties_input<S, D> { 
				special_control_provider,
				sane_defaults,
				prop_in.settings,
				extra_columns
			},
			[&parent_altered, &field_equality_predicate](const auto& modified, const unsigned index = -1) {
				auto addr = ::make_field_address(parent_altered, modified);
				addr.element_index = index;

				return field_equality_predicate(modified, addr);
			},
			[&parent_altered, &do_tweaker](const std::string& formatted_label, const tweaker_type t, const auto& modified) {
				const auto addr = ::make_field_address(parent_altered, modified);

				do_tweaker(tweaker_input<std::decay_t<decltype(modified)>>{
					t, formatted_label, addr, modified, modified
				});
			},
			member_label,
			member
		);
	};

	augs::introspect(traverse, parent_altered);
}

#pragma once
#include "augs/pad_bytes.h"
#include "augs/templates/traits/is_maybe.h"
#include "augs/templates/folded_finders.h"
#include "augs/string/string_templates_declaration.h"
#include "augs/drawing/flip.h"
#include "game/enums/filters.h"

#include "application/setups/editor/property_editor/property_editor_structs.h"
#include "application/setups/editor/property_editor/property_editor_settings.h"
#include "application/setups/editor/property_editor/default_widget_provider.h"
#include "application/setups/editor/property_editor/changes_describers.h"

#include "augs/string/format_enum.h"
#include "augs/misc/imgui/imgui_enum_combo.h"

#include "application/setups/editor/detail/maybe_different_colors.h"
#include "application/setups/editor/property_editor/tweaker_type.h"
#include "application/setups/editor/detail/field_address.h"
#include "augs/templates/introspection_utils/count_members.h"

#include "augs/templates/traits/is_enum_map.h"
#include "augs/misc/enum/enum_map.h"

#define HIDE_DISABLED_MAYBES 1

template <class T>
static constexpr bool forbid_zero_elements_v = 
	is_one_of_v<T, plain_animation_frames_type, std::vector<particles_emission>>
;

template <class T>
static constexpr bool inline_with_members_v = 
	is_one_of_v<T, plain_animation_frame, light_value_variation>
;

template <class T>
static constexpr bool has_direct_widget_v = 
	is_one_of_v<T, std::string, bool, vec2, vec2i, rgba, b2Filter>
	|| is_arithmetic_minmax_v<T>
	|| std::is_arithmetic_v<T>
	|| std::is_enum_v<T>
;

template <class T>
static constexpr bool always_skip_in_properties =
	is_padding_field_v<T>
	// Yet unsupported:
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
				return input_multiline_text<512>(identity_label, altered, 4);
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
	else if constexpr(std::is_same_v<M, b2Filter>) {
		using P = predefined_filter_type;

		P found_predefined = P::COUNT;

		augs::for_each_enum_except_bounds(
			[&](const P p) {
				if (filters[p] == altered) {
					found_predefined = p;
				}
			}
		);

		if (enum_combo(
			identity_label, 
			found_predefined, 
			found_predefined == P::COUNT ? "(Custom)" : ""
		)) {
			altered = filters[found_predefined];
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
	else if constexpr(is_arithmetic_minmax_v<M>) {
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
		static_assert(always_false_v<M>, "Unsupported widget type.");
	}

	return std::nullopt;
}

template <class I, class M>
struct tweaker_input {
	const tweaker_type type;
	const std::string& field_name;
	field_address<I> field_location;
	const M& new_value;
};

template <class S, class D>
struct detail_edit_properties_input {
	S special_widget_provider;
	D sane_defaults;
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
	|| has_direct_widget_v<T>
;

template <class S, class T>
std::optional<tweaker_type> handler_or_direct(S provider, const std::string& identity_label, T& altered) {
	if constexpr(S::template handles<T>) {
		return provider.handle(identity_label, altered);
	}
	else if constexpr(has_direct_widget_v<T>) {
		(void)provider;
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
	class Behaviour,
	bool pass_notifier_through,
	bool inline_properties,
	class S,
	class D,
	class Eq,
	class F,
	class T
>
void detail_general_edit_properties(
	detail_edit_properties_input<S, D> input,
	Eq equality_predicate,
	F notify_change_of,
	const std::string& label,
	T& altered,
	const bool nodify_introspected = true
) {
	(void)input; (void)equality_predicate; (void)notify_change_of; (void)label; (void)altered; (void)nodify_introspected;

	static constexpr bool should_skip = 
		always_skip_in_properties<T> 
		|| Behaviour::template should_skip<T>
	;

	if constexpr(!should_skip) {
		const auto formatted_label = format_field_name(label);
		const auto identity_label = "##" + label;

		if constexpr(inline_properties || do_handler_or_direct<S, T>) {
			if constexpr(inline_properties) {
				auto colors = ::maybe_different_value_cols(input.settings, !equality_predicate(altered, std::nullopt)); 

				if (auto result = handler_or_direct(input.special_widget_provider, identity_label, altered)) {
					notify_change_of(formatted_label, *result, altered, std::nullopt);
				}

				ImGui::PopItemWidth();
				ImGui::SameLine();
			}
			else {
				auto scope = bulleted_property_name(formatted_label, input.extra_columns);
				auto colors = ::maybe_different_value_cols(input.settings, !equality_predicate(altered, std::nullopt)); 

				if (auto result = handler_or_direct(input.special_widget_provider, identity_label, altered)) {
					notify_change_of(formatted_label, *result, altered, std::nullopt);
				}
			}
		}
		else {
			if constexpr(is_maybe_v<T>) {
				const auto all_equal = equality_predicate(altered.is_enabled, std::nullopt);

				{
					auto scope = bulleted_property_name(formatted_label, input.extra_columns);
					auto colors = ::maybe_different_value_cols(input.settings, !all_equal); 

					if (const auto result = augs::imgui::checkbox(identity_label, altered.is_enabled)) {
						notify_change_of(formatted_label, tweaker_type::DISCRETE, altered.is_enabled, std::nullopt);
					}
				}

				const bool value_enabled = all_equal && altered.is_enabled;
				const bool nodify = false;

#if HIDE_DISABLED_MAYBES
				if (value_enabled) {
					auto id = augs::imgui::scoped_id(formatted_label + "maybe");

					detail_general_edit_properties<Behaviour, pass_notifier_through, inline_properties>(input, equality_predicate, notify_change_of, label, altered.value, nodify);
				}
#else
				auto colors = maybe_disabled_cols(input.settings, !value_enabled);

				auto id = augs::imgui::scoped_id(formatted_label + "maybe");

				detail_general_edit_properties<Behaviour, pass_notifier_through, inline_properties>(input, equality_predicate, notify_change_of, label, altered.value, nodify);
#endif
			}
			else if constexpr(is_std_array_v<T> || is_enum_array_v<T>) {
				auto further = [&]() {
					for (unsigned i = 0; i < static_cast<unsigned>(altered.size()); ++i) {
						std::string element_label;

						if constexpr(is_enum_array_v<T>) {
							element_label = format_enum(static_cast<typename T::enum_type>(i));
						}
						else {
							element_label = std::to_string(i);
						}

						detail_general_edit_properties<Behaviour, pass_notifier_through, inline_properties>(input, equality_predicate, notify_change_of, element_label, altered[i]);
					}
				};

				if (nodify_introspected) {
					if (auto node = node_and_columns(formatted_label, input.extra_columns)) {
						further();
					}
				}
				else {
					auto ind = augs::imgui::scoped_indent();
					further();
				}
			}
			else if constexpr(is_enum_map_v<T>) {
				if (auto node = node_and_columns(formatted_label, input.extra_columns)) {
					using E = typename T::key_type;

					augs::for_each_enum_except_bounds(
						[&](const E i) {
							if (const auto value = mapped_or_nullptr(altered, i)) {
								auto i_id = augs::imgui::scoped_id(static_cast<int>(i));

								const auto element_label = format_enum(i);

								if constexpr(pass_notifier_through) {
									detail_general_edit_properties<Behaviour, true, inline_properties>(
										input, 
										equality_predicate,
										notify_change_of,
										element_label,
										*value
									);
								}
								else {
									detail_general_edit_properties<Behaviour, true, inline_properties>(
										input, 
										[&equality_predicate, i, &altered] (auto&&...) { return equality_predicate(altered, i); },
										[&notify_change_of, i, &altered] (const auto& l, const tweaker_type t, auto&&...) { notify_change_of(l, t, altered, i); },
										element_label,
										*value
									);
								}
							}
						}
					);
				}
			}
			else if constexpr(is_container_v<T>) {
				constexpr bool resizable = !has_constexpr_size_v<T>;
				constexpr bool has_size_limit = has_constexpr_max_size_v<T>;

				if constexpr(can_access_data_v<T>) {
					using V = typename T::value_type;

					constexpr bool should_skip_whole_container = 
						always_skip_in_properties<V> 
						|| Behaviour::template should_skip<V>
					;

					if constexpr(!should_skip_whole_container) {
						auto displayed_container_label = formatted_label;

						if constexpr(resizable && has_size_limit) {
							displayed_container_label = typesafe_sprintf(
								"%x (%x/%x)###%x", 
								formatted_label, 
								altered.size(), 
								altered.capacity(), 
								formatted_label
							);
						}

						if (auto node = node_and_columns(displayed_container_label, input.extra_columns)) {
							if constexpr(S::template handles_prologue<T>) {
								ImGui::NextColumn();

								if (input.special_widget_provider.handle_prologue(displayed_container_label, altered)) {
									notify_change_of(formatted_label, tweaker_type::DISCRETE, altered, std::nullopt);
								}

								augs::imgui::next_columns(input.extra_columns + 1);
							}

							for (unsigned i = 0; i < static_cast<unsigned>(altered.size()); ++i) {
								auto i_id = augs::imgui::scoped_id(i);

								if constexpr(resizable) {
									{
										auto colors = maybe_disabled_cols(input.settings, altered.size() >= altered.max_size());

										if (ImGui::Button("D")) {
											auto duplicated = altered.at(i);
											altered.insert(altered.begin() + i + 1, std::move(duplicated));
											notify_change_of(formatted_label, tweaker_type::DISCRETE, altered, std::nullopt);
											break;
										}

										ImGui::SameLine();

									}

									{
										auto colors = maybe_disabled_cols(
											input.settings, 
											altered.size() == 1 && forbid_zero_elements_v<T>
										);

										if (ImGui::Button("-")) {
											altered.erase(altered.begin() + i);
											notify_change_of(formatted_label, tweaker_type::DISCRETE, altered, std::nullopt);
											break;
										}
									}
								}

								ImGui::SameLine();

								const auto element_label = std::to_string(i);

								if constexpr(pass_notifier_through) {
									detail_general_edit_properties<Behaviour, true, inline_properties>(
										input, 
										equality_predicate,
										notify_change_of,
										element_label,
										altered.at(i)
									);
								}
								else {
									detail_general_edit_properties<Behaviour, true, inline_properties>(
										input, 
										[&equality_predicate, i, &altered] (auto&&...) { return equality_predicate(altered, i); },
										[&notify_change_of, i, &altered] (const auto& l, const tweaker_type t, auto&&...) { notify_change_of(l, t, altered, i); },
										element_label,
										altered.at(i)
									);
								}
							}

							if constexpr (resizable) {
								if (altered.size() < altered.max_size()) {
									if (ImGui::Button("+")) {
										altered.emplace_back(input.sane_defaults.template construct<typename T::value_type>());
										notify_change_of(formatted_label, tweaker_type::DISCRETE, altered, std::nullopt);
									}

									augs::imgui::next_columns(input.extra_columns + 2);
								}
							}
						}
					}
				}
			}
			else {
				auto further = [&](const std::string& l, auto& m) {
					detail_general_edit_properties<Behaviour, pass_notifier_through, inline_properties>(input, equality_predicate, notify_change_of, l, m);
				};

				if (nodify_introspected) {
					if constexpr(inline_with_members_v<T>) {
						augs::imgui::text(formatted_label);
						augs::imgui::next_columns(1);

						{
							auto whole_column = augs::imgui::scoped_item_width(-1);

							const auto num_members = static_cast<int>(augs::count_members(altered));
							ImGui::PushMultiItemsWidths(num_members);

							auto further_inline = [&](const std::string& l, auto& m) {
								detail_general_edit_properties<Behaviour, pass_notifier_through, true>(input, equality_predicate, notify_change_of, l, m);
							};

							augs::introspect(further_inline, altered);
						}

						augs::imgui::next_columns(1 + input.extra_columns);
					}
					else {
						if (auto node = node_and_columns(formatted_label, input.extra_columns)) {
							augs::introspect(further, altered);
						}
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

struct default_edit_properties_behaviour {
	template <class T>
	static constexpr bool should_skip = false;
};

template <
	class field_type_id,
	class Behaviour = default_edit_properties_behaviour,
   	class T,
   	class G,
   	class H,
	class Eq = true_returner,
	class S = default_widget_provider,
	class D = default_sane_default_provider
>
void general_edit_properties(
	const property_editor_input prop_in,
	T parent_altered,
	G post_new_change,
	H rewrite_last,
	Eq field_equality_predicate = {},
	S special_widget_provider = {},
	D sane_defaults = {},
	const int extra_columns = 0
) {
	using namespace augs::imgui;

	prop_in.state.poll_change_of_active_widget();

	auto do_tweaker = [&](const auto tw_in) {
		const auto type = tw_in.type;
		const auto& field_name = tw_in.field_name;
		const auto& field_location = tw_in.field_location;
		const auto& new_value = tw_in.new_value;

		if (type == tweaker_type::DISCRETE) {
			post_new_change(
				::describe_changed(field_name, new_value, special_widget_provider),
				field_location,
				new_value
			);
		}
		else if (type == tweaker_type::CONTINUOUS) {
			const auto description = ::describe_changed(field_name, new_value, special_widget_provider);

			if (prop_in.state.tweaked_widget_changed()) {
				post_new_change(description, field_location, new_value);
			}
			else {
				rewrite_last(description, new_value);
			}
		}
	};

	auto traverse = [&](const auto& member_label, auto& member) {
		detail_general_edit_properties<Behaviour, false, false>(
			detail_edit_properties_input<S, D> { 
				special_widget_provider,
				sane_defaults,
				prop_in.settings,
				extra_columns
			},
			[&parent_altered, &field_equality_predicate](const auto& modified, const auto index) {
				using I = std::remove_const_t<decltype(index)>;

				auto addr = ::make_field_address<field_type_id>(parent_altered, modified);

				if constexpr(std::is_same_v<I, unsigned>) {
					addr.element_index = index;
					return field_equality_predicate(modified.at(index), addr);
				}
				else {
					return field_equality_predicate(modified, addr);
				}
			},
			[&parent_altered, &do_tweaker](const std::string& formatted_label, const tweaker_type t, const auto& modified, const auto index) {
				using I = std::remove_const_t<decltype(index)>;

				auto addr = ::make_field_address<field_type_id>(parent_altered, modified);

				if constexpr(std::is_same_v<I, unsigned>) {
					addr.element_index = index;

					do_tweaker(tweaker_input<field_type_id, remove_cref<decltype(modified.at(index))>>{
						t, formatted_label, addr, modified.at(index)
					});
				}
				else {
					do_tweaker(tweaker_input<field_type_id, remove_cref<decltype(modified)>>{
						t, formatted_label, addr, modified
					});
				}
			},
			member_label,
			member
		);
	};

	augs::introspect(traverse, parent_altered);
}

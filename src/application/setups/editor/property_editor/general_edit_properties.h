#pragma once
#include "augs/pad_bytes.h"
#include "game/assets/ids/is_asset_id.h"
#include "augs/templates/traits/is_tuple.h"
#include "augs/templates/type_matching_and_indexing.h"
#include "augs/string/string_templates_declaration.h"
#include "augs/drawing/flip.h"

#include "application/setups/editor/editor_settings.h"
#include "application/setups/editor/property_editor/property_editor_structs.h"
#include "application/setups/editor/property_editor/property_editor_settings.h"

#include "augs/string/format_enum.h"
#include "augs/misc/imgui/imgui_enum_combo.h"

template <class A, class B>
auto describe_changed_flag(
	const A& field_name,
	const B& new_value
) {
	return typesafe_sprintf(
		"%x %x", 
		new_value ? "Set" : "Unset", 
		field_name
	);
};

template <class A, class B>
auto describe_changed_generic(
	const A& field_name,
	const B& new_value
) {
	if constexpr(std::is_enum_v<B>) {
		return typesafe_sprintf(
			"Set %x to %x",
			field_name,
			format_enum(new_value)
		);
	}
	else {
		return typesafe_sprintf(
			"Set %x to %x",
			field_name,
			new_value
		);
	}
};

template <class A, class B>
description_pair describe_changed(
	const A& field_name,
	const B& old_value,
   	const B& new_value
) {
	using F = std::decay_t<decltype(new_value)>;

	if constexpr(std::is_same_v<F, bool>) {
		return { "", describe_changed_flag(field_name, new_value) };
	}
	else {
		if constexpr(std::is_same_v<F, std::string>) {
			if (field_name == "Name") {
				return { 
					typesafe_sprintf("Renamed %x ", old_value),
					typesafe_sprintf("to %x ", new_value)
				};
			}
		}

		return { "", describe_changed_generic(
			field_name,
			new_value
		) };
	}
};


template <class F, class Eq>
auto maybe_different_value_cols(
	const property_editor_settings& settings,
	const F& first,
   	const field_address field_id,
   	Eq& pred
) {
	using namespace augs::imgui;

	const bool values_different = !pred.compare(first, field_id);

	return std::make_tuple(
		cond_scoped_style_color(values_different, ImGuiCol_FrameBg, settings.different_values_frame_bg),
		cond_scoped_style_color(values_different, ImGuiCol_FrameBgHovered, settings.different_values_frame_hovered_bg),
		cond_scoped_style_color(values_different, ImGuiCol_FrameBgActive, settings.different_values_frame_active_bg)
	);
};

template <
	template <class T> class SkipPredicate = always_false,
   	class T,
   	class G,
   	class H,
	class Eq
>
void general_edit_properties(
	const property_editor_input in,
	const T& object,
	G post_new_change_impl,
	H rewrite_last_change_impl,
	Eq field_equality_predicate
) {
	using namespace augs::imgui;

	auto& old_description = in.state.old_description;

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

	{
		auto& last_active = in.state.last_active;

		if (last_active && last_active.value() != ImGui::GetActiveID()) {
			last_active.reset();
		}
	}

	augs::introspect(
		augs::recursive([&](auto self, const std::string& original_label, const auto& original_member) {
			using M = std::decay_t<decltype(original_member)>;

			static constexpr bool should_skip = 
				is_padding_field_v<M> 
				|| SkipPredicate<M>::value
			;

			if constexpr(!should_skip) {
				const auto field = [&](){
					field_address result;

					result.offset = static_cast<unsigned>(
						reinterpret_cast<const std::byte*>(std::addressof(original_member))
						- reinterpret_cast<const std::byte*>(std::addressof(object))
					);

					auto& id = result.type_id;

					if constexpr(std::is_trivially_copyable_v<M>) {
						id.set<augs::trivial_type_marker>();
					}
					else if constexpr(is_container_v<M>) {
						id.set<M>();
					}
					else {
						static_assert(has_introspect_v<M>);
					}

					return result;
				}();

				auto post_new = [&](
					const description_pair& description,
					const auto& new_content
				) {
					post_new_change(description, field, new_content);
				};

				auto do_maybe_different_value_cols = [&](auto... args) {
					return maybe_different_value_cols(in.settings, original_member, field, field_equality_predicate, args...);
				};

				auto handle_continuous_tweaker = [&](
					const auto& field_name,
					const auto& old_value, 
					const auto& new_value,
					auto callback,
					auto... args
				) {
					const auto colors = do_maybe_different_value_cols(args...);

					if (callback()) {
						const auto this_id = ImGui::GetActiveID();
						const auto description = describe_changed(field_name, old_value, new_value);

						auto& last_active = in.state.last_active;

						if (last_active != this_id) {
							/* LOG("Started dragging %x to %x", field_name, new_value); */
							post_new(description, new_value);
						}
						else {
							/* LOG("Dragging %x to %x", field_name, new_value); */
							rewrite_last(description.of_new, new_value);
						}

						last_active = this_id;
					}
				};

				auto handle_discrete_tweaker = [&](
					const auto& field_name,
					const auto& old_value, 
					const auto& new_value,
					auto callback,
				   	auto... args
				) {
					const auto colors = do_maybe_different_value_cols(args...);

					if (callback()) {
						post_new(
							describe_changed(field_name, old_value, new_value),
							new_value
						);
					}
				};

				const auto label = format_field_name(original_label);
				const auto identity_label = "##" + label;

				auto altered_member = original_member;

				auto do_tweaker = [&](auto f) {
					ImGui::Bullet();
					text(label);
					ImGui::NextColumn();
					auto scope = scoped_item_width(-1);
					f();
					ImGui::NextColumn();
				};

				auto do_continuous = [&](auto f, auto... args) {
					do_tweaker([&]() { 
						handle_continuous_tweaker(label, original_member, altered_member, f, args...); 
					});
				};

				auto do_discrete = [&](auto f, auto... args) {
					do_tweaker([&]() { 
						handle_discrete_tweaker(label, original_member, altered_member, f, args...);
					});
				};

				if constexpr(is_padding_field_v<M>) {
					return;	
				}
				else if constexpr(std::is_same_v<M, b2Filter>) {
					// TODO: checkbox matrix
					return;
				}
				else if constexpr(std::is_same_v<M, std::string>) {
					do_continuous([&]() { 
						if (original_label == "description") {
							return input_multiline_text<512>(identity_label, altered_member, 8);
						}
						else if (original_label == "name") {
							return input_text<256>(identity_label, altered_member);
						}

						return input_text<256>(identity_label, altered_member);
					});

					/* next_column_text(); */
				}
				else if constexpr(std::is_same_v<M, bool>) {
					do_discrete([&]() { 
						return checkbox(identity_label, altered_member);
					});
				}
				else if constexpr(is_container_v<M>) {

				}
				else if constexpr(std::is_arithmetic_v<M>) {
					do_continuous([&]() { 
						return drag(identity_label, altered_member); 
					});

					/* next_column_text(get_type_name<M>()); */
				}
				else if constexpr(is_one_of_v<M, vec2, vec2i>) {
					do_continuous([&]() { 
						return drag_vec2(identity_label, altered_member); 
					});

					/* next_column_text(); */
				}
				else if constexpr(is_minmax_v<M>) {
					do_continuous([&]() { 
						return drag_minmax(identity_label, altered_member); 
					});

					/* next_column_text(get_type_name<typename M::first_type>() + " range"); */
				}
				else if constexpr(is_asset_id_v<M>) {

				}
				else if constexpr(std::is_enum_v<M>) {
					do_discrete([&]() { 
						return enum_combo(identity_label, altered_member);
					});

					/* next_column_text(); */
				}
				else if constexpr(std::is_same_v<M, rgba>) {
					do_continuous([&]() { 
						return color_edit(identity_label, altered_member);
					});

					/* next_column_text(); */
				}
				else {
					const auto object_node = scoped_tree_node_ex(label);

#if 0
					if constexpr(is_tuple_v<M>) {
						next_column_text("Tuple");
					}
					else {
						next_column_text(get_type_name<M>());
					}
#endif
					next_column_text();

					if (object_node) {
						augs::introspect(augs::recursive(self), original_member);
					}
				}
			}
		}),
		object
	);
}

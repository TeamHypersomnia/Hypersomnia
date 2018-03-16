#pragma once
#include "augs/pad_bytes.h"
#include "augs/templates/type_matching_and_indexing.h"
#include "augs/drawing/flip.h"
#include "application/setups/editor/editor_command_structs.h"
#include "application/setups/editor/property_editor_structs.h"

template <class T>
decltype(auto) get_name_of(const entity_flavour<T>& flavour) {
	return flavour.template get<invariants::name>().name;
}

template <class T>
auto get_invariant_stem(const T&) {
	auto result = format_field_name(get_type_name_strip_namespace<T>());
	result[0] = std::toupper(result[0]);

	/* These two look ugly with automated names */

	if constexpr(std::is_same_v<T, invariants::sprite>) {
		result = "Sprite";
	}	

	if constexpr(std::is_same_v<T, invariants::polygon>) {
		result = "Polygon";
	}

	return result;
}

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
	return typesafe_sprintf(
		"Set %x to %x",
		field_name,
		new_value
	);
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
		if (field_name == "Name") {
			return { 
				typesafe_sprintf("Renamed %x ", old_value),
				typesafe_sprintf("to %x ", new_value)
			};
		}

		return { "", describe_changed_generic(
			field_name,
			new_value
		) };
	}
};

inline void next_column_text(const std::string& tx = "") {
	ImGui::NextColumn();

	if (tx.size() > 0) {
		augs::imgui::text(tx);
	}

	ImGui::NextColumn();
};

inline void next_column_text_disabled(const std::string& tx = "") {
	ImGui::NextColumn();

	if (tx.size() > 0) {
		augs::imgui::text_disabled(tx);
	}

	ImGui::NextColumn();
};

template <class T, class F, class G, class H>
void general_edit_properties(
	editor_properties_gui& state,
	const T& object,
	F make_property_id,
	G post_new_change_impl,
	H rewrite_last_change_impl
) {
	using namespace augs::imgui;

	auto& old_description = state.old_description;

	auto post_new_change = [&](
		const description_pair& description,
		const auto property_id,
		const auto& new_content
	) {
		old_description = description.of_old;
		const auto new_description = old_description + description.of_new;

		post_new_change_impl(new_description, property_id, new_content);
	};

	auto rewrite_last = [&](
		const auto& description,
		const auto& new_content
	) {
		rewrite_last_change_impl(old_description + description, new_content);
	};

	{
		auto& last_active = state.last_active;

		if (last_active && last_active.value() != ImGui::GetActiveID()) {
			last_active.reset();
		}
	}

	augs::introspect(
		augs::recursive([&](auto self, const std::string& original_label, const auto& original_member) {
			using M = std::decay_t<decltype(original_member)>;

			const auto current_offset = static_cast<unsigned>(
				reinterpret_cast<const std::byte*>(std::addressof(original_member))
				- reinterpret_cast<const std::byte*>(std::addressof(object))
			);

			const auto current_type_id = [&](){
				edited_field_type_id id;

				if constexpr(std::is_trivially_copyable_v<M>) {
					id.set<augs::trivial_type_marker>();
				}
				else if constexpr(is_container_v<M>) {
					id.set<M>();
				}
				else {
					static_assert(has_introspect_v<M>);
				}

				return id;
			}();

			const auto property_id = make_property_id(
				current_offset,
				current_type_id
			);

			auto post_new = [&](
				const description_pair& description,
				const auto& new_content
			) {
				post_new_change(description, property_id, new_content);
			};

			auto handle_continuous_tweaker = [&](
				const auto& field_name,
	   			const auto& old_value, 
				const auto& new_value,
	   			auto callback
			) {
				if (callback()) {
					const auto this_id = ImGui::GetActiveID();
					const auto description = describe_changed(field_name, old_value, new_value);
		
					auto& last_active = state.last_active;

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
	   			auto callback
			) {
				if (callback()) {
					post_new(
						describe_changed(field_name, old_value, new_value),
						new_value
					);
				}
			};

			const auto label = format_field_name(original_label);
			auto altered_member = original_member;

			auto do_continuous = [&](auto&& f) {
				handle_continuous_tweaker(label, original_member, altered_member, std::forward<decltype(f)>(f));
			};

			auto do_discrete = [&](auto&& f) {
				handle_discrete_tweaker(label, original_member, altered_member, std::forward<decltype(f)>(f));
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
						return input_multiline_text<512>("Description", altered_member, 16);
					}
					else if (original_label == "name") {
						return input_text<256>("Name", altered_member);
					}

					return input_text<256>(label, altered_member);
				});

				next_column_text();
			}
			else if constexpr(std::is_same_v<M, flip_flags>) {
				auto do_flag = [&](const auto& flag_name, auto& flag) {
					if (checkbox(flag_name, flag)) {
						post_new(
							{ "", describe_changed_flag(flag_name, flag) },
							altered_member
						);
					}

					next_column_text();
				};

				do_flag("Flip horizontally", altered_member[0]);
				do_flag("Flip vertically", altered_member[1]);
			}
			else if constexpr(std::is_same_v<M, bool>) {
				do_discrete([&]() { 
					return checkbox(label, altered_member);
				});

				next_column_text();
			}
			else if constexpr(is_container_v<M>) {

			}
			else if constexpr(std::is_arithmetic_v<M>) {
				do_continuous([&]() { 
					return drag(label, altered_member); 
				});

				next_column_text(get_type_name<M>());
			}
			else if constexpr(is_one_of_v<M, vec2, vec2i>) {
				do_continuous([&]() { 
					return drag_vec2(label, altered_member); 
				});

				next_column_text();
			}
			else if constexpr(is_minmax_v<M>) {
				do_continuous([&]() { 
					return drag_minmax(label, altered_member); 
				});

				next_column_text(get_type_name<typename M::first_type>() + " range");
			}
			else if constexpr(std::is_enum_v<M>) {

			}
			else if constexpr(std::is_same_v<M, rgba>) {
				do_continuous([&]() { 
					return color_edit(label, altered_member);
				});

				next_column_text();
			}
			else {
				const auto object_node = scoped_tree_node_ex(label.c_str());

				next_column_text(get_type_name<M>());

				if (object_node) {
					augs::introspect(augs::recursive(self), original_member);
				}
			}
		}),
		object
	);
}

template <class T, class E>
void edit_invariant(
	editor_properties_gui& state,
	const T& invariant,
	const entity_flavour<E>& source_flavour,
	const typed_entity_flavour_id<E> flavour_id,
   	const editor_command_input in
) {
	using namespace augs::imgui;

	auto make_property_id = [&](
		const auto offset,
		const auto type_id
	) {
		flavour_property_id result;

		result.flavour_id = flavour_id;
		result.invariant_id = index_in_list_v<T, decltype(entity_flavour<E>::invariants)>;

		result.field_offset = offset;
		result.field_type = type_id;

		return result;
	};

	const auto property_location = [&]() {
		const auto flavour_name = get_name_of(source_flavour);
		const auto invariant_name = get_invariant_stem(invariant);

		return typesafe_sprintf("(in %x of %x)", invariant_name, flavour_name);
	}();

	/* Linker error fix */
	auto& history = in.folder.history;

	auto post_new_change = [&](
		const auto& description,
		const auto property_id,
		const auto& new_content
	) {
		change_flavour_property_command cmd;
		cmd.property_id = property_id;

		cmd.value_after_change = augs::to_bytes(new_content);
		cmd.built_description = description + property_location;

		history.execute_new(cmd, in);
	};

	auto rewrite_last_change = [&](
		const auto& description,
		const auto& new_content
	) {
		auto& last = history.last_command();

		if (auto* const cmd = std::get_if<change_flavour_property_command>(std::addressof(last))) {
			cmd->built_description = description + property_location;
			cmd->rewrite_change(augs::to_bytes(new_content), in);
		}
		else {
			LOG("WARNING! There was some problem with tracking activity of editor controls.");
		}
	};

	general_edit_properties(
		state, 
		invariant,
		make_property_id,
		post_new_change,
		rewrite_last_change
	);
}

template <class E>
void edit_flavour(
	editor_properties_gui& state,
	const typed_entity_flavour_id<E> flavour_id,
	const entity_flavour<E>& flavour,
   	const editor_command_input in
) {
	using namespace augs::imgui;

	edit_invariant(state, flavour.template get<invariants::name>(), flavour, flavour_id, in);

	for_each_through_std_get(
		flavour.invariants,
		[&](auto& invariant) {
			using T = std::decay_t<decltype(invariant)>;

			if constexpr(std::is_same_v<T, invariants::name>) {
				/* This one is handled already */
				return;
			}

			const auto invariant_label = get_invariant_stem(invariant) + " invariant";

			if (const auto node = scoped_tree_node_ex(invariant_label.c_str())) {
				edit_invariant(state, invariant, flavour, flavour_id, in);
			}
	
			next_column_text();
		}
   	);
}

#include "augs/misc/simple_pair.h"
#include "augs/templates/for_each_std_get.h"

#include "augs/readwrite/memory_stream.h"

#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/intercosm.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/gui/editor_all_entities_gui.h"

#include "application/setups/editor/gui/editor_properties_gui.h"

#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/byte_readwrite.h"

using namespace augs::imgui;

static auto details_text = [](const std::string& tx = ""){
	ImGui::NextColumn();

	if (tx.size() > 0) {
		text(tx);
	}

	ImGui::NextColumn();
};

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

template <class T>
decltype(auto) get_name_of(const entity_flavour<T>& flavour) {
	return flavour.template get<invariants::name>().name;
}

template <class T, class E>
void edit_invariant(
	const T& invariant,
	const entity_flavour<E>& source_flavour,
	const typed_entity_flavour_id<E> flavour_id,
   	const editor_command_input in
) {
	thread_local std::optional<ImGuiID> last_active;
	thread_local std::string cached_old_description = "";

	auto& history = in.folder.history;

	auto current_offset = static_cast<unsigned>(-1);
	auto current_type_id = edited_field_type_id();

	auto make_property_id = [&](){
		flavour_property_id result;
		result.field_offset = current_offset;
		result.field_type = current_type_id;
		result.invariant_id = index_in_list_v<T, decltype(entity_flavour<E>::invariants)>;
		result.flavour_id = flavour_id;

		return result;
	};

	const auto describe_property_location = [&](){
		const auto flavour_name = get_name_of(source_flavour);
		const auto invariant_name = get_invariant_stem(invariant);

		return typesafe_sprintf("(in %x of %x)", invariant_name, flavour_name);
	};

	auto describe_changed_flag = [&](
		const auto& field_name,
		const auto& new_value
	) {
		return typesafe_sprintf(
			"%x %x %x", 
			new_value ? "Set" : "Unset", 
			field_name,
			describe_property_location()
		);
	};

	auto describe_changed_generic = [&](
		const auto& field_name,
		const auto& new_value
	) {
		return typesafe_sprintf(
			"Set %x to %x %x",
			field_name,
			new_value,
			describe_property_location()
		);
	};

	struct description_pair {
		std::string of_old;
		std::string of_new;
	};

	auto describe_changed = [&](
		const auto& field_name,
		const auto& old_value,
	   	const auto& new_value
	) -> description_pair {
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

	/* Linker error fix */
	auto& old_description = cached_old_description;

	auto post_new_change = [&](
		const description_pair& description,
	   	const auto& new_content
	) mutable {
		old_description = description.of_old;

		change_flavour_property_command cmd;
		cmd.property_id = make_property_id();

		cmd.value_after_change = augs::to_bytes(new_content);
		cmd.built_description = old_description + description.of_new;
		history.execute_new(cmd, in);
	};

	auto rewrite_last_change = [&](
		const auto& description,
		const auto& new_content
	) {
		auto& last = history.last_command();

		if (auto* const cmd = std::get_if<change_flavour_property_command>(std::addressof(last))) {
			cmd->built_description = old_description + description;
			cmd->rewrite_change(augs::to_bytes(new_content), in);
		}
		else {
			LOG("WARNING! There was some problem with tracking activity of editor controls.");
		}
	};

	if (last_active && last_active.value() != ImGui::GetActiveID()) {
		/* LOG("Released"); */
		last_active.reset();
	}

	auto handle_continuous_tweaker = [&](
		const auto& field_name,
	   	const auto& old_value, 
		const auto& new_value,
	   	auto callback
	) {
		if (callback()) {
			const auto this_id = ImGui::GetActiveID();
			const auto description = describe_changed(field_name, old_value, new_value);

			if (last_active != this_id) {
				/* LOG("Started dragging %x to %x", field_name, new_value); */
				post_new_change(description, new_value);
			}
			else {
				/* LOG("Dragging %x to %x", field_name, new_value); */
				rewrite_last_change(description.of_new, new_value);
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
			post_new_change(
				describe_changed(field_name, old_value, new_value),
				new_value
			);
		}
	};

	augs::introspect(
		augs::recursive([&](auto self, const std::string& original_label, const auto& original_member) {
			using M = std::decay_t<decltype(original_member)>;

			current_offset = static_cast<unsigned>(
				reinterpret_cast<const std::byte*>(std::addressof(original_member))
				- reinterpret_cast<const std::byte*>(std::addressof(invariant))
			);
			
			if constexpr(std::is_trivially_copyable_v<M>) {
				current_type_id.set<augs::trivial_type_marker>();
			}
			else if constexpr(is_container_v<M>) {
				current_type_id.set<M>();
			}
			else {
				static_assert(has_introspect_v<M>);
			}

			auto altered_member = original_member;

			const auto label = format_field_name(original_label);

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

				details_text();
			}
			else if constexpr(std::is_same_v<M, flip_flags>) {
				auto do_flag = [&](const auto& flag_name, auto& flag) {
					if (checkbox(flag_name, flag)) {
						post_new_change(
							{ "", describe_changed_flag(flag_name, flag) },
							altered_member
						);
					}

					details_text();
				};

				do_flag("Flip horizontally", altered_member[0]);
				do_flag("Flip vertically", altered_member[1]);
			}
			else if constexpr(std::is_same_v<M, bool>) {
				do_discrete([&]() { 
					return checkbox(label, altered_member);
				});

				details_text();
			}
			else if constexpr(is_container_v<M>) {

			}
			else if constexpr(std::is_arithmetic_v<M>) {
				do_continuous([&]() { 
					return drag(label, altered_member); 
				});

				details_text(get_type_name<M>());
			}
			else if constexpr(is_one_of_v<M, vec2, vec2i>) {
				do_continuous([&]() { 
					return drag_vec2(label, altered_member); 
				});

				details_text();
			}
			else if constexpr(is_minmax_v<M>) {
				do_continuous([&]() { 
					return drag_minmax(label, altered_member); 
				});

				details_text(get_type_name<typename M::first_type>() + " range");
			}
			else if constexpr(std::is_enum_v<M>) {

			}
			else if constexpr(std::is_same_v<M, rgba>) {
				do_continuous([&]() { 
					return color_edit(label, altered_member);
				});

				details_text();
			}
			else {
				const auto object_node = scoped_tree_node_ex(label.c_str());

				details_text(get_type_name<M>());

				if (object_node) {
					augs::introspect(augs::recursive(self), original_member);
				}
			}
		}),
		invariant
	);
}

template <class E>
void edit_flavour(
	const typed_entity_flavour_id<E> flavour_id,
	const entity_flavour<E>& flavour,
   	const editor_command_input in
) {
	edit_invariant(flavour.template get<invariants::name>(), flavour, flavour_id, in);

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
				edit_invariant(invariant, flavour, flavour_id, in);
			}
	
			ImGui::NextColumn();
			ImGui::NextColumn();
		}
   	);
}

void editor_all_entities_gui::open() {
	show = true;
	acquire_once = true;
	ImGui::SetWindowFocus("All entities");
}

void editor_all_entities_gui::perform(const editor_command_input in) {
	if (!show) {
		return;
	}

	using namespace augs::imgui;

	auto entities = scoped_window("All entities", &show);
	auto& work = *in.folder.work;
	auto& cosm = work.world;

	if (acquire_once) {
		ImGui::SetKeyboardFocusHere();
		acquire_once = false;
	}

	thread_local ImGuiTextFilter filter;
	filter.Draw();

	ImGui::Columns(2); // 4-ways, with border
	ImGui::NextColumn();
	text_disabled("Details");
	ImGui::NextColumn();
	ImGui::Separator();

	cosm.get_solvable().for_each_pool(
		[&](const auto& p){
			using P = decltype(p);
			using pool_type = std::decay_t<P>;

			using E = type_argument_t<typename pool_type::mapped_type>;

			const auto entity_type_label = format_field_name(get_type_name<E>());
			const auto total_entities = p.size();
			const auto total_flavours = cosm.get_common_significant().get_flavours<E>().count();

			ImGui::SetNextTreeNodeOpen(true, ImGuiCond_FirstUseEver);

			const auto node = scoped_tree_node_ex(entity_type_label.c_str());

			ImGui::NextColumn();
			text_disabled(typesafe_sprintf("%x Flavours, %x Entities", total_flavours, total_entities));
			ImGui::NextColumn();

			if (node) {
				cosm.change_common_significant([&](cosmos_common_significant& common_signi){
					common_signi.get_flavours<E>().for_each(
						[&](
							const auto flavour_id,
							/* 
								Note: we accept flavour as const, 
								because ImGUI itself should only see the immutable reference.

							   	Is the job of the change_flavour_property_command to actually alter flavour state.
							*/
						   	const auto& flavour
						){
							const auto flavour_label = flavour.template get<invariants::name>().name;

							if (!filter.PassFilter(flavour_label.c_str())) {
								return;
							}

							const auto all_having_flavour = cosm.get_solvable_inferred().name.get_entities_by_flavour_id(flavour_id);

							const auto node_label = typesafe_sprintf("%x###%x", flavour_label, flavour_id.raw);
							const auto f_node = scoped_tree_node_ex(node_label.c_str());

							ImGui::NextColumn();
							text_disabled(typesafe_sprintf("%x Entities", all_having_flavour.size()));
							ImGui::NextColumn();

							if (f_node) {
								ImGui::Separator();
								edit_flavour(flavour_id, flavour, in);
								ImGui::Separator();

								for (const auto& e : all_having_flavour) {
									bool s = false;
									ImGui::Selectable(typesafe_sprintf("%x", cosm[e].get_guid()).c_str(), &s);

									ImGui::NextColumn();
									ImGui::NextColumn();
								}
							}
						}
					);

					return changer_callback_result::DONT_REFRESH;
				});
			}
		}	
	);
}

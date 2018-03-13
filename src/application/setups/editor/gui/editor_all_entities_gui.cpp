#include "augs/templates/for_each_std_get.h"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/intercosm.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/gui/editor_all_entities_gui.h"

#include "application/setups/editor/gui/editor_properties_gui.h"

using namespace augs::imgui;

static auto details_text = [](const auto& tx){
	ImGui::NextColumn();
	text(tx);
	ImGui::NextColumn();
};

template <class T>
void edit_properties_of(T& object, const editor_command_input in) {
	auto& history = in.folder.history;

	augs::introspect(
		[&](const std::string& original_label, auto& member) {
			using M = std::decay_t<decltype(member)>;

			const auto label = format_field_name(original_label);

			auto id = scoped_id(std::addressof(member));
			if constexpr(is_padding_field_v<M>) {
				return;	
			}
			else if constexpr(std::is_same_v<M, std::string>) {
			}
			else if constexpr(std::is_same_v<M, flip_flags>) {
				checkbox("Flip horizontally", member[0]);
				checkbox("Flip vertically", member[1]);
			}
			else if constexpr(std::is_same_v<M, bool>) {
				checkbox(label, member);
			}
			else if constexpr(is_container_v<M>) {

			}
			else if constexpr(std::is_arithmetic_v<M>) {
				drag(label, member);
				details_text(get_type_name<M>());
			}
			else if constexpr(std::is_enum_v<M>) {

			}
			else if constexpr(std::is_same_v<M, rgba>) {
				color_edit(label, member);
				details_text(get_type_name<M>());
			}
			else {
				const auto object_node = scoped_tree_node_ex(label.c_str());

				details_text(get_type_name<M>());

				if (object_node) {
					edit_properties_of(member, in);
				}
			}
		},
		object
	);
}

template <class E>
void edit_flavour(
	entity_flavour<E>& object,
   	const editor_command_input in
) {
	input_text<256>("Name", object.template get<invariants::name>().name, ImGuiInputTextFlags_EnterReturnsTrue);
	ImGui::NextColumn();
	ImGui::NextColumn();
	input_multiline_text<256>("Description", object.template get<invariants::name>().description, 4);
	ImGui::NextColumn();
	ImGui::NextColumn();

	for_each_through_std_get(
		object.invariants,
		[&](auto& invariant) {
			using T = std::decay_t<decltype(invariant)>;

			const auto invariant_label = [](){
				auto result = format_field_name(get_type_name_strip_namespace<T>()) + " invariant";
				result[0] = std::toupper(result[0]);

				/* These two look ugly with automated names */

				if constexpr(std::is_same_v<T, invariants::sprite>) {
					result = "Sprite invariant";
				}	

				if constexpr(std::is_same_v<T, invariants::polygon>) {
					result = "Polygon invariant";
				}

				return result;
			}();

			if (const auto node = scoped_tree_node_ex(invariant_label.c_str())) {
				edit_properties_of(invariant, in);
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
						[&](const auto flavour_id, auto& flavour){
							const auto flavour_label = flavour.template get<invariants::name>().name;

							if (!filter.PassFilter(flavour_label.c_str())) {
								return;
							}

							const auto all_having_flavour = cosm.get_solvable_inferred().name.get_entities_by_flavour_id(flavour_id);

							const auto f_node = scoped_tree_node_ex(flavour_label.c_str());
							ImGui::NextColumn();
							text_disabled(typesafe_sprintf("%x Entities", all_having_flavour.size()));
							ImGui::NextColumn();

							if (f_node) {
								ImGui::Separator();
								edit_flavour(flavour, in);
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

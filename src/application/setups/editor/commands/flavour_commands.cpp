#include "augs/string/string_templates.h"
#include "application/setups/editor/commands/flavour_commands.h"
#include "application/setups/editor/editor_folder.h"
#include "application/intercosm.h"
#include "application/setups/editor/commands/asset_commands.h"

#include "application/setups/editor/property_editor/widgets/pathed_asset_widget.h"

std::string create_flavour_command::describe() const {
	return built_description;
}

void create_flavour_command::redo(const editor_command_input in) {
	redo_and_copy(in, {});
}

void create_flavour_command::redo_and_copy(const editor_command_input in, const raw_entity_flavour_id source) {
	type_id.dispatch(
		[&](auto e) {
			using E = decltype(e);
			using flavour_type = entity_flavour<E>;

			static const auto entity_type_label = str_ops(format_field_name(get_type_name<E>())).replace_all(" ", "").subject;

			auto& work = *in.folder.work;
			auto& cosm = work.world;
			auto& defs = work.viewables;

			const auto source_flavour = cosm.find_flavour(typed_entity_flavour_id<E>(source));

			const auto basic_name = [source_flavour]() {
				if (source_flavour) {
					return source_flavour->get_name();
				}

				return entity_type_label;
			}();

			auto& flavours = cosm.get_flavours<E>({});
			auto& new_object = base::redo(flavours);

			if (source_flavour != nullptr) {
				new_object = *source_flavour;
			}
			else {
				if constexpr(flavour_type::template has<invariants::sprite>()) {
					const auto provider = asset_sane_default_provider { defs };
					new_object.set(provider.construct<invariants::sprite>());
				}
			}

			auto make_new_flavour_name = [&](const auto i) {
				return basic_name + "-" + std::to_string(i);
			};

			for (int i = 1; ; ++i) {
				const auto tried_name = make_new_flavour_name(i);
				bool is_free = true;

				for_each_id_and_object(flavours,
					[&](const auto&, const auto& flavour) {
						if (flavour.get_name() == tried_name) {
							is_free = false;
						}
					}
				);

				if (is_free) {
					new_object.template get<invariants::name>().name = tried_name;

					if (source_flavour) {
						built_description = typesafe_sprintf("Duplicated flavour: %x", source_flavour->get_name());
					}
					else {
						built_description = typesafe_sprintf("Created flavour: %x", tried_name);
					}

					break;
				}
			}
		}
	);
}

void create_flavour_command::undo(const editor_command_input in) {
	type_id.dispatch(
		[&](auto e) {
			using E = decltype(e);

			auto& work = *in.folder.work;
			auto& cosm = work.world;
			auto& flavours = cosm.get_flavours<E>({});

			base::undo(flavours);
		}
	);
}

void duplicate_flavour_command::redo(const editor_command_input in) {
	base::redo_and_copy(in, duplicate_from);
}

void duplicate_flavour_command::undo(const editor_command_input in) {
	base::undo(in);
}

void delete_flavour_command::redo(const editor_command_input in) {
	type_id.dispatch(
		[&](auto e) {
			using E = decltype(e);

			auto& work = *in.folder.work;
			auto& cosm = work.world;

			auto& flavours = cosm.get_flavours<E>({});
			
			built_description = "Deleted flavour: " + flavours.get(freed_id).get_name();

			base::redo(flavours);
		}
	);
}

void delete_flavour_command::undo(const editor_command_input in) {
	type_id.dispatch(
		[&](auto e) {
			using E = decltype(e);

			auto& work = *in.folder.work;
			auto& cosm = work.world;

			auto& flavours = cosm.get_flavours<E>({});
			
			base::undo(flavours);
		}
	);
}

std::string delete_flavour_command::describe() const {
	return built_description;
}

std::string instantiate_flavour_command::describe() const {
	return built_description;
}

void instantiate_flavour_command::redo(const editor_command_input in) {
	ensure(!created_id.is_set());

	instantiated_id.type_id.dispatch(
		[&](auto e) {
			using E = decltype(e);

			const auto flavour_id = typed_entity_flavour_id<E>(instantiated_id.raw);

			auto& work = *in.folder.work;
			auto& cosm = work.world;

			const auto& flavour = cosm.get_flavour(flavour_id);

			built_description = typesafe_sprintf("Instantiated flavour: %x", flavour.get_name());

			created_id = cosmic::specific_create_entity(cosm, flavour_id, [](const auto){}).get_id();
		}
	);
}

void instantiate_flavour_command::undo(const editor_command_input in) {
	auto& work = *in.folder.work;
	auto& cosm = work.world;

	cosmic::undo_last_create_entity(cosm[created_id]);
	created_id = {};
}

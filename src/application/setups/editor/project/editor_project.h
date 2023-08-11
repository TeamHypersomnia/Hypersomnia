#pragma once
#include "augs/misc/constant_size_string.h"
#include "application/setups/editor/project/editor_project_meta.h"
#include "application/setups/editor/project/editor_project_about.h"
#include "application/setups/editor/project/editor_arena_settings.h"
#include "application/setups/editor/project/editor_layers.h"

#include "application/setups/editor/project/editor_playtesting_settings.h"

#include "application/setups/editor/nodes/editor_node_pools.h"
#include "application/setups/editor/resources/editor_resource_pools.h"

template <class E>
struct editor_typed_node_id;

template <class E>
struct editor_typed_resource_id;

struct editor_node_id;
struct editor_resource_id;

/*
	Note that meta is always the first - 
	so, to identify the game version string of a cached project binary file (and thus determine if the rest is safe to read),
	you will always have to just read the firstmost bytes into game_version_identifier.
*/

struct editor_official_resource_map;

using name_to_node_map_type = std::unordered_map<std::string, editor_node_id>;

struct parent_layer_info {
	editor_layer_id layer_id;
	const editor_layer* layer_ptr = nullptr;
	std::size_t layer_index;
	std::size_t index_in_layer;
};

struct editor_project {
	// GEN INTROSPECTOR struct editor_project
	editor_project_meta meta;
	editor_project_about about;

	editor_arena_settings settings;
	editor_playtesting_settings playtesting;

	editor_node_pools nodes;
	editor_resource_pools resources;

	editor_layers layers;
	// END GEN INTROSPECTOR

	/*
		Unbacked resource is one whose file on disk is missing.
		Missing resource is an unbacked resource that is referenced by a current revision.

		An unbacked resource that is not used anywhere will *not* reported as missing.
		We need to keep track of all unbacked resources ever seen however, because redoing or undoing editor commands
		might cause an unbacked resource to become missing (e.g. undoing a delete of the only node referencing that resource)
	*/

	std::vector<editor_resource_id> last_unbacked_resources;
	std::vector<editor_resource_id> last_missing_resources;

	template <typename T>
	auto& get() {
		if constexpr(std::is_same_v<editor_project_about, T>) {
			return about;
		}
		else if constexpr(std::is_same_v<editor_arena_settings, T>) {
			return settings;
		}
		else if constexpr(std::is_same_v<editor_playtesting_settings, T>) {
			return playtesting;
		}
		else {
			static_assert(always_false_v<T>, "Non-exhaustive if constexpr");
		}
	}

	auto& get_game_modes() {
		return resources.get_pool_for<editor_game_mode_resource>();
	}

	const auto& get_game_modes() const {
		return resources.get_pool_for<editor_game_mode_resource>();
	}

	using O = editor_resource_pools;

	template <class T>
	T* find_resource(O& officials, const editor_typed_resource_id<T>& id);

	template <class T>
	const T* find_resource(const O& officials, const editor_typed_resource_id<T>& id) const;

	template <class F>
	decltype(auto) on_resource(O& officials, const editor_resource_id& id, F&& callback);

	template <class F>
	decltype(auto) on_resource(const O& officials, const editor_resource_id& id, F&& callback) const;

	template <class T>
	T* find_node(const editor_node_id& id);

	template <class T>
	const T* find_node(const editor_node_id& id) const;

	template <class T>
	T* find_node(const editor_typed_node_id<T>& id);

	template <class T>
	const T* find_node(const editor_typed_node_id<T>& id) const;

	template <class F>
	decltype(auto) on_node(const editor_node_id& id, F&& callback);

	template <class F>
	decltype(auto) on_node(const editor_node_id& id, F&& callback) const;

	auto make_find_resource_lambda(const O& officials) const;
	auto make_on_resource_lambda(const O& officials) const;

	editor_layer* find_layer(const editor_layer_id& id);
	const editor_layer* find_layer(const editor_layer_id& id) const;

	editor_layer* find_layer(const std::string& name);
	editor_layer_id find_layer_id(const std::string& name) const;

	bool recount_references(const O& officials, bool recount_officials) const;
	bool mark_changed_resources(const editor_official_resource_map& officials_map) const;

	bool rescan_resources_to_track(const O& officials, const editor_official_resource_map& officials_map) const;

	template <class R, class F>
	void for_each_resource(F&& callback) const;

	name_to_node_map_type make_name_to_node_map() const;

	std::optional<parent_layer_info> find_parent_layer(editor_node_id id) const;
	std::optional<parent_layer_info> convert_to_parent_layer_info(editor_layer_id id) const;

	void clear_cached_scene_node_data();

private:
	template <class S, class Officials, class F>
	static decltype(auto) on_resource_impl(S& self, Officials& officials, const editor_resource_id& id, F&& callback);

	template <class S, class Officials, class T>
	static decltype(auto) find_resource_impl(S& self, Officials& officials, const editor_typed_resource_id<T>& id);
};

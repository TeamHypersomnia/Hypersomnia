#pragma once
#include "augs/misc/imgui/standard_window_mixin.h"
#include "application/setups/editor/nodes/editor_node_id.h"
#include "application/setups/editor/resources/editor_resource_id.h"
#include "application/setups/editor/project/editor_layers.h"
#include "application/setups/editor/gui/inspected_variant.h"
#include "augs/filesystem/path_declaration.h"

enum class inspected_node_tab_type {
	NODE,
	RESOURCE
};

class editor_setup;

struct editor_inspector_input {
	editor_setup& setup;
};

class editor_tweaked_widget_tracker {
	struct tweak_session {
		ImGuiID id;
		std::size_t command_index;
		bool operator==(const tweak_session&) const = default;
	};

	std::optional<tweak_session> last_tweaked;

public:
	void poll_change(std::size_t current_command_index);
	bool changed(std::size_t current_command_index) const;
	void update(std::size_t current_command_index);
	void reset();
};

struct editor_inspector_gui : standard_window_mixin<editor_inspector_gui> {
	using base = standard_window_mixin<editor_inspector_gui>;
	using base::base;
	using introspect_base = base;

	inspected_node_tab_type node_current_tab = inspected_node_tab_type::NODE;
	editor_tweaked_widget_tracker tweaked_widget;

	template <class T>
	bool inspects_any() const {
		for (const auto& inspected : all_inspected) {
			if (std::get_if<T>(std::addressof(inspected)) != nullptr) {
				return true;
			}
		}

		return false;
	}

	template <class T>
	bool inspects_any_different_than() const {
		for (const auto& inspected : all_inspected) {
			if (std::get_if<T>(std::addressof(inspected)) == nullptr) {
				return true;
			}
		}

		return false;
	}

	template <class T>
	bool inspects_only() const {
		return !inspects_any_different_than<T>();
	}

	template <class T>
	const T* get_first_inspected() const {
		for (const auto& inspected : all_inspected) {
			if (const auto typed = std::get_if<T>(std::addressof(inspected))) {
				return typed;
			}
		}

		return nullptr;
	}

	template <class T, class F>
	void for_each_inspected(F&& callback) const {
		for (const auto inspected : all_inspected) {
			if (const auto typed = std::get_if<T>(std::addressof(inspected))) {
				callback(*typed);
			}
		}
	}

	template <class T>
	auto get_all_inspected() const {
		std::vector<T> result;

		for_each_inspected<T>([&](const T& typed) { result.push_back(typed); });

		return result;
	}

	std::vector<inspected_variant> all_inspected;

	void inspect(inspected_variant, bool wants_multiple);
	void perform(editor_inspector_input);

	void clear() {
		all_inspected.clear();
	}

private:
	friend editor_setup;

	using order = std::pair<std::size_t, std::size_t>;
	using ordered_inspected = std::vector<std::pair<order, inspected_variant>>;

	ordered_inspected cached_orders;
	augs::path_type reveal_in_explorer_once;

	auto& prepare_for_sorting() {
		cached_orders.clear();

		const auto n = all_inspected.size();
		cached_orders.resize(n);

		for (std::size_t i = 0; i < n; ++i) {
			cached_orders[i] = { { 0, 0 }, all_inspected[i] };
		}

		return cached_orders;
	}

	void set_from(const ordered_inspected& ordered) {
		all_inspected.clear();

		for (auto& o : ordered) {
			all_inspected.emplace_back(o.second);
		}
	}
};


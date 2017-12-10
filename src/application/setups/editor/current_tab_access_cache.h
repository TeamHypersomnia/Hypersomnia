#pragma once
#include <cstdlib>
#include "augs/build_settings/platform_defines.h"

struct editor_tab;
struct intercosm;
using tab_index_type = unsigned;

template <class derived>
class current_tab_access_cache {
	editor_tab* current_tab = nullptr;
	intercosm* current_work = nullptr;

protected:
	tab_index_type current_index = static_cast<tab_index_type>(-1);

	void refresh() {
		auto& self = *static_cast<derived*>(this);

		if (current_index != static_cast<tab_index_type>(-1)) {
			current_tab = &self.tabs[current_index];
			current_work = self.works[current_index].get();
		}
	}

	void set_current_tab(const tab_index_type i) {
		auto& self = *static_cast<derived*>(this);

		if (current_index != i) {
			self.on_tab_changed();
		}

		current_index = i;
		refresh();
	}

	void unset_current_tab() {
		current_index = static_cast<tab_index_type>(-1);

		current_tab = nullptr;
		current_work = nullptr;
	}

	FORCE_INLINE bool has_current_tab() const {
		return current_tab != nullptr;
	}

	FORCE_INLINE auto& tab() {
		return *current_tab;
	}

	FORCE_INLINE const auto& tab() const {
		return *current_tab;
	}

	FORCE_INLINE auto& work() {
		return *current_work;
	}

	FORCE_INLINE const auto& work() const {
		return *current_work;
	}
};

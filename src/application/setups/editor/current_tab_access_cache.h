#pragma once
#include <cstdlib>
#include "augs/build_settings/platform_defines.h"

struct editor_tab;
struct workspace;

template <class derived>
class current_tab_access_cache {
	editor_tab* current_tab = nullptr;
	workspace* current_work = nullptr;

protected:
	std::size_t current_index = -1;

	void set_current_tab(const std::size_t i) {
		auto& self = *static_cast<derived*>(this);

		current_index = i;

		current_tab = &self.tabs[i];
		current_work = self.works[i].get();
	}

	void unset_current_tab() {
		current_index = -1;

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

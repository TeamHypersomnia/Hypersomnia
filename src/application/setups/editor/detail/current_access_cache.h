#pragma once
#include <cstdlib>
#include "augs/build_settings/platform_defines.h"

struct editor_view;
struct editor_folder;

struct intercosm;
using folder_index = unsigned;

template <class derived>
class current_access_cache {
	editor_folder* current_folder = nullptr;
	intercosm* current_work = nullptr;

protected:
	void refresh() {
		auto& self = *static_cast<derived*>(this);

		auto& folders = self.signi.folders;

		if (folders.empty() || self.signi.current_index == static_cast<folder_index>(-1)) {
			self.signi.current_index = static_cast<folder_index>(-1);

			current_folder = nullptr;
			current_work = nullptr;

			return;
		}

		current_folder = &folders[self.signi.current_index];
		current_work = current_folder->work.get();
	}

	void set_current(const folder_index i) {
		auto& self = *static_cast<derived*>(this);

		if (self.signi.current_index != i) {
			self.on_folder_changed();
		}

		self.signi.current_index = i;
		refresh();
	}

	FORCE_INLINE bool anything_opened() const {
		return current_folder != nullptr;
	}

	FORCE_INLINE auto& work() {
		return *current_work;
	}

	FORCE_INLINE const auto& work() const {
		return *current_work;
	}

	FORCE_INLINE auto& player() {
		return current_folder->player;
	}

	FORCE_INLINE const auto& player() const {
		return current_folder->player;
	}

	FORCE_INLINE auto& folder() {
		return *current_folder;
	}

	FORCE_INLINE const auto& folder() const {
		return *current_folder;
	}

	FORCE_INLINE auto& view() {
		return folder().view;
	}

	FORCE_INLINE const auto& view() const {
		return folder().view;
	}
};

#pragma once

template <class E>
bool is_like_plantable_bomb(const E& self) {
	if (const auto fuse = self.template find<components::hand_fuse>()) {
		if (const auto fuse_def = self.template find<invariants::hand_fuse>()) {
			return fuse_def->is_like_plantable_bomb();
		}
	}

	return false;
}

template <class E>
bool is_like_planted_bomb(const E& self) {
	if (const auto fuse = self.template find<components::hand_fuse>()) {
		if (const auto fuse_def = self.template find<invariants::hand_fuse>()) {
			return fuse->armed() && fuse_def->is_like_plantable_bomb();
		}
	}

	return false;
}

template <class E>
bool is_like_planted_or_defused_bomb(const E& self) {
	if (const auto fuse = self.template find<components::hand_fuse>()) {
		if (const auto fuse_def = self.template find<invariants::hand_fuse>()) {
			return (fuse->defused() || fuse->armed()) && fuse_def->is_like_plantable_bomb();
		}
	}

	return false;
}

template <class E>
bool is_like_thrown_explosive(const E& self) {
	if (const auto fuse = self.template find<components::hand_fuse>()) {
		if (const auto fuse_def = self.template find<invariants::hand_fuse>()) {
			return !fuse_def->is_like_plantable_bomb() && fuse->armed() && self.get_owning_transfer_capability().dead();
		}
	}

	return false;
}


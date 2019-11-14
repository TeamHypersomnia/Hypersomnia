#pragma once

template <bool C, class ModeAndRulesType>
class basic_arena_handle;

template <class A>
auto get_faction_of(const A& handle, const mode_player_id& id) {
	return handle.on_mode(
		[&](const auto& typed_mode) -> std::optional<faction_type> {
			const auto entry = typed_mode.find(id);

			if (entry == nullptr) {
				return std::nullopt;
			}

			return entry->get_faction();
		}
	);
}

template <class A>
bool is_spectator(const A& handle, const mode_player_id& id) {
	const auto faction = ::get_faction_of(handle, id);
	return faction == std::nullopt || faction == faction_type::SPECTATOR;
}

template <bool C, class M>
bool found_in(const basic_arena_handle<C, M>& handle, const mode_player_id& id) {
	return handle.on_mode(
		[&](const auto& typed_mode) {
			return typed_mode.find(id) != nullptr;
		}
	);
}

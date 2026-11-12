#pragma once
#include <cstdint>
#include <type_traits>
#include "game/modes/test_mode.h"

template <class ArenaHandle, class Rebuild>
void rebuild_keeping_sound_clock(ArenaHandle arena, Rebuild&& rebuild) {
	const auto sound_clock = arena.on_mode(
		[]<typename M>(const M& typed_mode) {
			if constexpr(std::is_same_v<M, test_mode>) {
				return typed_mode.sound_clock;
			}
			else {
				return static_cast<uint32_t>(0);
			}
		}
	);

	rebuild();

	arena.on_mode(
		[sound_clock]<typename M>(M& typed_mode) {
			if constexpr(std::is_same_v<M, test_mode>) {
				typed_mode.sound_clock = sound_clock;
			}
		}
	);
}

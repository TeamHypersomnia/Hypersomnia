#pragma once
#include "game/cosmos/cosmos.h"
#include "test_scenes/test_scene_flavours.h"
#include "test_scenes/create_test_scene_entity.h"

class test_scene_node {
	template <class F>
	decltype(auto) on_enum(F&& f) const {
		return std::visit(std::forward<F>(f), enum_id);
	}

	cosmos* cosm;

	std::variant<
		test_plain_sprited_bodies,
		test_complex_decorations,
		test_sprite_decorations,
		test_sound_decorations,
		test_particles_decorations
	> enum_id;
public:
	test_scene_node() = default;
	test_scene_node(const test_scene_node&) = default;

	template <class E>
	test_scene_node(cosmos& csm, const E e) : 
		cosm(&csm),
		enum_id(e)
	{}

	flip_flags flip;
	real32 rotation = 0.f;

	template <class E>
	auto next(const E e) const {
		auto cloned = *this;
		cloned.enum_id = e;
		return cloned;
	}

	auto next() const {
		auto cloned = *this;
		return cloned;
	}

	auto get_size() const {
		return on_enum([&](const auto id) {
			const auto& f = ::get_test_flavour(cosm->get_common_significant().flavours, id);

			if constexpr(remove_cref<decltype(f)>::template has<invariants::sprite>()) {
				return f.template get<invariants::sprite>().size;
			}
			else {
				return vec2i(5, 5);
			}
		});
	}

	void create(const vec2 resolved_pos, const vec2i resolved_size) const {
		on_enum([&](const auto id) {
			create_test_scene_entity(*cosm, id, [&](const auto typed_handle, auto&&...) {
				typed_handle.set_logic_transform(transformr(resolved_pos, rotation));
				typed_handle.do_flip(flip);

				if (resolved_size != get_size()) {
					typed_handle.set_logical_size(resolved_size);
				}
			});
		});
	}
};



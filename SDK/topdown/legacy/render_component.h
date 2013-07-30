#pragma once
#include "../../../entity_system/entity_system.h"

struct renderable;
namespace components {
	struct render : public augmentations::entity_system::component {
		unsigned layer;

		struct mask {
			unsigned id;
			bool cullable;
		};

		static const mask WORLD;
		static const mask GUI;
		static const mask SCREEN_EFFECT;

		const mask render_mask;

		renderable* instance;
		render(unsigned layer, renderable* instance, mask render_mask = WORLD)
			: layer(layer), render_mask(render_mask), instance(instance) {}
	};
	const render::mask render::GUI = { 1, false };
	const render::mask render::WORLD = { 0, true };
	const render::mask render::SCREEN_EFFECT = { 2, false };
}
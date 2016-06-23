#pragma once
#include "all.h"
#include "game/resources/manager.h"
#include "graphics/shader.h"

namespace resource_setups {
	void load_standard_everything() {
		resource_setups::load_standard_atlas();
		resource_setups::load_standard_particle_effects();
		resource_setups::load_standard_behaviour_trees();

		resource_manager.create(assets::shader_id::DEFAULT_VERTEX, L"hypersomnia/shaders/default.vsh", augs::graphics::shader::type::VERTEX);
		resource_manager.create(assets::shader_id::DEFAULT_FRAGMENT, L"hypersomnia/shaders/default.fsh", augs::graphics::shader::type::FRAGMENT);
		resource_manager.create(assets::program_id::DEFAULT, assets::shader_id::DEFAULT_VERTEX, assets::shader_id::DEFAULT_FRAGMENT);

		resource_manager.create(assets::shader_id::DEFAULT_HIGHLIGHT_VERTEX, L"hypersomnia/shaders/default_highlight.vsh", augs::graphics::shader::type::VERTEX);
		resource_manager.create(assets::shader_id::DEFAULT_HIGHLIGHT_FRAGMENT, L"hypersomnia/shaders/default_highlight.fsh", augs::graphics::shader::type::FRAGMENT);
		resource_manager.create(assets::program_id::DEFAULT_HIGHLIGHT, assets::shader_id::DEFAULT_HIGHLIGHT_VERTEX, assets::shader_id::DEFAULT_HIGHLIGHT_FRAGMENT);

		resource_manager.create(assets::shader_id::CIRCULAR_BARS_VERTEX, L"hypersomnia/shaders/circular_bars.vsh", augs::graphics::shader::type::VERTEX);
		resource_manager.create(assets::shader_id::CIRCULAR_BARS_FRAGMENT, L"hypersomnia/shaders/circular_bars.fsh", augs::graphics::shader::type::FRAGMENT);
		resource_manager.create(assets::program_id::CIRCULAR_BARS, assets::shader_id::CIRCULAR_BARS_VERTEX, assets::shader_id::CIRCULAR_BARS_FRAGMENT);
	}
}
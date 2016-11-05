#include "all.h"
#include "game/resources/manager.h"
#include "augs/graphics/shader.h"

namespace resource_setups {
	void load_standard_everything() {
		resource_setups::load_standard_atlas();
		resource_setups::load_standard_particle_effects();
		resource_setups::load_standard_behaviour_trees();

		resource_manager.create(assets::shader_id::DEFAULT_VERTEX, "hypersomnia/shaders/default.vsh", augs::graphics::shader::type::VERTEX);
		resource_manager.create(assets::shader_id::DEFAULT_FRAGMENT, "hypersomnia/shaders/default.fsh", augs::graphics::shader::type::FRAGMENT);
		resource_manager.create(assets::program_id::DEFAULT, assets::shader_id::DEFAULT_VERTEX, assets::shader_id::DEFAULT_FRAGMENT);

		resource_manager.create(assets::shader_id::PURE_COLOR_HIGHLIGHT_VERTEX, "hypersomnia/shaders/pure_color_highlight.vsh", augs::graphics::shader::type::VERTEX);
		resource_manager.create(assets::shader_id::PURE_COLOR_HIGHLIGHT_FRAGMENT, "hypersomnia/shaders/pure_color_highlight.fsh", augs::graphics::shader::type::FRAGMENT);
		resource_manager.create(assets::program_id::PURE_COLOR_HIGHLIGHT, assets::shader_id::PURE_COLOR_HIGHLIGHT_VERTEX, assets::shader_id::PURE_COLOR_HIGHLIGHT_FRAGMENT);

		resource_manager.create(assets::shader_id::CIRCULAR_BARS_VERTEX, "hypersomnia/shaders/circular_bars.vsh", augs::graphics::shader::type::VERTEX);
		resource_manager.create(assets::shader_id::CIRCULAR_BARS_FRAGMENT, "hypersomnia/shaders/circular_bars.fsh", augs::graphics::shader::type::FRAGMENT);
		resource_manager.create(assets::program_id::CIRCULAR_BARS, assets::shader_id::CIRCULAR_BARS_VERTEX, assets::shader_id::CIRCULAR_BARS_FRAGMENT);
	}
}
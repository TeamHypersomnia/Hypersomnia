#include "application/setups/editor/official/create_official_resources.h"
#include "augs/string/string_templates.h"
#include "augs/templates/type_map.h"
#include "augs/templates/introspect.h"
#include "augs/string/string_templates_declaration.h"
#include "augs/misc/pool/pool_allocate.h"

using to_resource_type_t = type_map<
	type_pair<official_sprites, editor_sprite_resource>,
	type_pair<official_sounds, editor_sound_resource>,
	type_pair<official_lights, editor_light_resource>
>;

template <class E>
auto& create_official(E id, editor_resource_pools& pools) {
	using R = to_resource_type_t::at<E>;
	auto& pool = pools.template get_pool_for<R>();

	auto& new_object = pool.allocate().object;
	new_object.unique_name = to_lowercase(augs::enum_to_string(id));

	return new_object;
}

void create_lights(editor_resource_pools& pools) {
	{
		auto& strong_lamp = create_official(official_lights::STRONG_LAMP, pools);
		(void)strong_lamp;
	}

	{
		auto& aquarium_lamp = create_official(official_lights::AQUARIUM_LAMP, pools).editable;
		aquarium_lamp.attenuation.constant = 75;
		aquarium_lamp.attenuation.quadratic = 631;
	}
}

void create_official_resources(editor_resource_pools& pools) {
	create_lights(pools);
}

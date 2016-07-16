#include "entity_handle.h"
#include "game/components/render_component.h"
#include "game/components/substance_component.h"
#include "game/components/processing_component.h"
#include "game/components/dynamic_tree_node_component.h"
#include "game/components/special_physics_component.h"

#include "game/cosmos.h"
#include "game/detail/physics_scripts.h"

typedef cosmos O;
typedef put_all_components_into<augs::component_aggregate>::type N;

template <bool C>
template <class>
void augs::basic_handle<C, O, N>::add_standard_components() {
	if (has<components::transform>() && has<components::physics>())
		get<components::physics>().set_transform(get<components::transform>());

	if (has<components::render>() && !is_entity_physical(*this) && !has<components::dynamic_tree_node>())
		add(components::dynamic_tree_node::get_default(*this));

	if (has<components::physics>() && !has<components::special_physics>())
		add(components::special_physics());

	add(components::processing::get_default(*this));
	add(components::substance());
}

// explicit instantiation
template void augs::basic_handle<false, cosmos, put_all_components_into<augs::component_aggregate>::type>::add_standard_components();